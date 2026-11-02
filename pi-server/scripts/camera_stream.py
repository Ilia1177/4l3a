#!/usr/bin/env python3
"""Serve the Raspberry Pi camera as an MJPEG stream.

The libcamera V4L2 compatibility layer (libcamerify) is unreliable with
ustreamer, so this bridge reads the MJPEG stream that rpicam-vid already
produces and re-serves it over HTTP.

Endpoints:
  /stream    multipart/x-mixed-replace MJPEG
  /snapshot  latest JPEG frame
  /state     JSON status
"""

import json
import os
import signal
import subprocess
import sys
import threading
import time
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer

HOST = os.environ.get("CAMERA_HOST", "127.0.0.1")
PORT = int(os.environ.get("CAMERA_PORT", "8081"))
WIDTH = int(os.environ.get("CAMERA_WIDTH", "640"))
HEIGHT = int(os.environ.get("CAMERA_HEIGHT", "480"))
FPS = int(os.environ.get("CAMERA_FPS", "12"))
QUALITY = int(os.environ.get("CAMERA_QUALITY", "80"))
BOUNDARY = "frame"
SOI = b"\xff\xd8"
EOI = b"\xff\xd9"

_stop = threading.Event()
_camera = None


def log(message):
    print(f"[camera-stream] {message}", file=sys.stderr, flush=True)


def flag_supported(flag):
    try:
        result = subprocess.run(
            ["rpicam-vid", "--help"],
            capture_output=True,
            text=True,
            timeout=5,
        )
    except Exception:
        return False
    return flag in (result.stdout + result.stderr)


class Camera:
    def __init__(self):
        self._cond = threading.Condition()
        self._frame = None
        self._seq = 0
        self._last_frame_at = 0.0
        self._clients = 0
        self._proc = None
        self._cmd = self._build_command()
        self._thread = threading.Thread(target=self._run, name="camera", daemon=True)

    def _build_command(self):
        cmd = [
            "rpicam-vid",
            "-t", "0",
            "--codec", "mjpeg",
            "--width", str(WIDTH),
            "--height", str(HEIGHT),
            "--framerate", str(FPS),
            "--quality", str(QUALITY),
            "--nopreview",
        ]
        if flag_supported("--flush"):
            cmd.append("--flush")
        cmd.extend(["-o", "-"])
        return cmd

    def start(self):
        self._thread.start()

    def stop(self):
        _stop.set()
        self.wake()
        proc = self._proc
        if proc is not None and proc.poll() is None:
            proc.terminate()
            try:
                proc.wait(timeout=3)
            except subprocess.TimeoutExpired:
                proc.kill()

    def wake(self):
        with self._cond:
            self._cond.notify_all()

    def add_client(self):
        with self._cond:
            self._clients += 1

    def remove_client(self):
        with self._cond:
            self._clients = max(0, self._clients - 1)

    def snapshot(self):
        with self._cond:
            return self._frame

    def wait_frame(self, last_seq, timeout=10.0):
        with self._cond:
            self._cond.wait_for(
                lambda: self._seq != last_seq or _stop.is_set(),
                timeout=timeout,
            )
            if self._seq == last_seq:
                return None, last_seq
            return self._frame, self._seq

    def state(self):
        with self._cond:
            now = time.monotonic()
            age = round(now - self._last_frame_at, 2) if self._last_frame_at else None
            online = age is not None and age < 5.0
            return {
                "online": online,
                "frames": self._seq,
                "clients": self._clients,
                "resolution": {"width": WIDTH, "height": HEIGHT},
                "desired_fps": FPS,
                "last_frame_age": age,
            }

    def _publish(self, frame):
        with self._cond:
            self._frame = frame
            self._seq += 1
            self._last_frame_at = time.monotonic()
            self._cond.notify_all()

    def _consume(self, buffer):
        while True:
            start = buffer.find(SOI)
            if start < 0:
                return buffer[-1:] if buffer.endswith(b"\xff") else b""
            buffer = buffer[start:]
            end = buffer.find(EOI, 2)
            if end < 0:
                return buffer
            frame = buffer[:end + 2]
            buffer = buffer[end + 2:]
            self._publish(frame)

    def _run(self):
        while not _stop.is_set():
            proc = None
            try:
                log("starting rpicam-vid")
                proc = subprocess.Popen(
                    self._cmd,
                    stdout=subprocess.PIPE,
                    stderr=None,
                    bufsize=0,
                )
                self._proc = proc
                buffer = b""
                while not _stop.is_set():
                    chunk = proc.stdout.read(65536)
                    if not chunk:
                        break
                    buffer = self._consume(buffer + chunk)
                if not _stop.is_set():
                    log(f"rpicam-vid exited with code {proc.poll()}")
            except FileNotFoundError:
                log("rpicam-vid not found")
                _stop.wait(5)
            except Exception as exc:
                log(f"camera error: {exc}")
            finally:
                self._proc = None
                if proc is not None and proc.poll() is None:
                    proc.kill()
                    try:
                        proc.wait(timeout=2)
                    except subprocess.TimeoutExpired:
                        pass
            if not _stop.is_set():
                _stop.wait(2)


class Handler(BaseHTTPRequestHandler):
    protocol_version = "HTTP/1.0"
    server_version = "CameraStream/1.0"

    def log_message(self, fmt, *args):
        log(f"{self.address_string()} {fmt % args}")

    def do_GET(self):
        path = self.path.split("?", 1)[0]
        if path == "/stream":
            self._stream()
        elif path == "/snapshot":
            self._snapshot()
        elif path == "/state":
            self._write_json(_camera.state())
        else:
            self.send_error(404)

    def _write_json(self, payload, status=200):
        body = json.dumps(payload).encode("utf-8")
        self.send_response(status)
        self.send_header("Content-Type", "application/json")
        self.send_header("Content-Length", str(len(body)))
        self.send_header("Cache-Control", "no-store")
        self.end_headers()
        self.wfile.write(body)

    def _snapshot(self):
        frame = _camera.snapshot()
        if frame is None:
            self._write_json({"error": "no frame yet"}, status=503)
            return
        self.send_response(200)
        self.send_header("Content-Type", "image/jpeg")
        self.send_header("Content-Length", str(len(frame)))
        self.send_header("Cache-Control", "no-store")
        self.end_headers()
        self.wfile.write(frame)

    def _stream(self):
        self.send_response(200)
        self.send_header(
            "Content-Type",
            f"multipart/x-mixed-replace; boundary={BOUNDARY}",
        )
        self.send_header("Cache-Control", "no-store")
        self.send_header("Pragma", "no-cache")
        self.send_header("Connection", "close")
        self.end_headers()

        _camera.add_client()
        last_seq = -1
        try:
            while not _stop.is_set():
                frame, seq = _camera.wait_frame(last_seq)
                if frame is None or seq == last_seq:
                    continue
                last_seq = seq
                header = (
                    f"--{BOUNDARY}\r\n"
                    f"Content-Type: image/jpeg\r\n"
                    f"Content-Length: {len(frame)}\r\n\r\n"
                ).encode("ascii")
                self.wfile.write(header)
                self.wfile.write(frame)
                self.wfile.write(b"\r\n")
                self.wfile.flush()
        except (BrokenPipeError, ConnectionResetError, OSError):
            pass
        finally:
            _camera.remove_client()


class Server(ThreadingHTTPServer):
    daemon_threads = True
    allow_reuse_address = True


def main():
    global _camera
    _camera = Camera()
    _camera.start()

    server = Server((HOST, PORT), Handler)

    def shutdown(signum, _frame):
        log(f"signal {signum}, shutting down")
        _stop.set()
        _camera.wake()
        threading.Thread(target=server.shutdown, daemon=True).start()

    signal.signal(signal.SIGTERM, shutdown)
    signal.signal(signal.SIGINT, shutdown)

    log(f"serving on http://{HOST}:{PORT}")
    try:
        server.serve_forever(poll_interval=0.5)
    finally:
        _camera.stop()
        server.server_close()
        log("stopped")


if __name__ == "__main__":
    main()

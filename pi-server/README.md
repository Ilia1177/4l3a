# pi-server

C++ HTTP server (cpp-httplib) serving `public/` on `127.0.0.1:8080`, meant to
sit behind Caddy which handles TLS for `4l3a.duckdns.org`.

## Build (on the Pi)

```bash
sudo apt update
sudo apt install -y build-essential cmake git

cd pi-server
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
```

The first build downloads cpp-httplib and GoogleTest via CMake FetchContent,
so the Pi needs internet access at build time (not at run time).

## Tests

```bash
make test           # C++ tests (CTest) + frontend tests (node:test)
make test-frontend  # frontend tests only
make check          # format, lint, build, tests, sanitizers
```

Frontend tests use `node --test` and require Node 18 or newer. The vendored
`src/minitel/Minitel1B_Hard.{h,cpp}` file is excluded from project format and
lint checks. `clang-tidy` is discovered on `PATH` or through Homebrew LLVM; if
needed, pass its path explicitly:

```bash
make CLANG_TIDY=/path/to/clang-tidy check
```

## Run manually (for testing)

```bash
cd build
./server
```

Visit `http://<pi-local-ip>:8080` directly to test before wiring up Caddy.
Watch `visits.log` in the `build/` folder, and the console output, to see the
`onClientVisit()` hook firing on each request.

## Install as a systemd service

```bash
# edit systemd/pi-server.service first: fix User= and paths if not "pi"
sudo cp systemd/pi-server.service /etc/systemd/system/
sudo systemctl daemon-reload
sudo systemctl enable --now pi-server
sudo systemctl status pi-server
journalctl -u pi-server -f   # tail logs
```

## Caddy

```bash
sudo apt install -y caddy   # if not already installed
sudo cp systemd/Caddyfile /etc/caddy/Caddyfile
sudo systemctl reload caddy
```

Make sure your router forwards external ports 80 and 443 to the Pi, and that
DuckDNS is kept updated with your public IP.

## Camera stream (Raspberry Pi only)

The page embeds an MJPEG stream at `/camera/stream`. A small Python bridge
(`scripts/camera_stream.py`) runs `rpicam-vid` and re-serves its MJPEG output
over HTTP on `127.0.0.1:8081`. Caddy strips the `/camera/` prefix and proxies
the result to the bridge. The C++ server does not touch the camera, and the old
`ustreamer` service is no longer used.

Endpoints exposed by the bridge (and therefore through Caddy):

| Path | Response |
|---|---|
| `/camera/stream` | `multipart/x-mixed-replace` MJPEG stream |
| `/camera/snapshot` | latest JPEG frame, or `503` before the first frame |
| `/camera/state` | JSON status: `online`, `frames`, `clients`, `resolution`, `desired_fps`, `last_frame_age` |

These steps are done **on the Pi** (the dev machine only stores the files).

Requirements: `python3` and `rpicam-vid` from `rpicam-apps`. Confirm the camera
is detected:

```bash
rpicam-vid --help >/dev/null && echo "rpicam-vid OK"
rpicam-hello --list-cameras
```

Test the bridge manually before installing the service:

```bash
python3 scripts/camera_stream.py &
curl -s http://127.0.0.1:8081/state
curl -s -o /tmp/snapshot.jpg http://127.0.0.1:8081/snapshot
file /tmp/snapshot.jpg   # JPEG image data, 640x480
```

The bridge only implements `GET`; `HEAD` returns `501`.

Install the service (adjust `User=` and paths if they differ):

```bash
sudo cp systemd/camera-stream.service /etc/systemd/system/
sudo systemctl daemon-reload
sudo systemctl enable --now camera-stream
systemctl status camera-stream
journalctl -u camera-stream -f   # tail logs
```

`camera-stream.service` declares `Conflicts=ustreamer.service`, so the old
streamer must not be running. Disable it if it is still enabled:
`sudo systemctl disable --now ustreamer`.

Reload Caddy with `systemd/Caddyfile`, then verify the public URL. The
`handle_path /camera/*` directive is required so the bridge receives `/state`
rather than `/camera/state`:

```bash
curl -s  https://<host>/camera/state
curl -s -o /tmp/snapshot.jpg -w '%{http_code} %{content_type}\n' https://<host>/camera/snapshot
curl -s --max-time 5 https://<host>/camera/stream -o /dev/null -w '%{http_code} %{content_type}\n'
```

Expected: `state` reports `"online": true`; `snapshot` returns `200
image/jpeg`; `stream` returns `200 multipart/x-mixed-replace; boundary=frame`.
The stream request stays open until the client disconnects, so always cap it
with `--max-time` when testing.

The bridge stays bound to `127.0.0.1:8081`; only Caddy exposes it over TLS.
Tune it with `CAMERA_WIDTH`, `CAMERA_HEIGHT`, `CAMERA_FPS` and `CAMERA_QUALITY`
(defaults `640`, `480`, `12`, `80`). MJPEG bandwidth is high (roughly 4–10 Mbps
per viewer at 640x480/12 fps).

If `rpicam-vid` fails, `journalctl -u camera-stream` shows its stderr. Confirm
`rpicam-hello --list-cameras` detects the sensor before changing anything else.

## Where to add your custom action

Edit `onClientVisit()` in `src/main.cpp`. It currently logs to console and to
`visits.log`, and bumps an in-memory counter exposed at `/api/status`. Add
GPIO control, webhook calls, database writes, etc. there — see the comments
in that function for a few concrete starting points.

If your action takes any real time (network calls, GPIO with delays, etc.),
dispatch it to a `std::thread` or a small task queue instead of running it
inline, so it doesn't add latency to every page load.

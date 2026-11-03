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

The first build downloads cpp-httplib via CMake FetchContent, so the Pi
needs internet access at build time (not at run time).

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

The page embeds an MJPEG stream at `/camera/stream`. Caddy proxies that path to
`ustreamer`, which reads the CSI camera through libcamera. The C++ server does
not touch the camera.

These steps are done **on the Pi** (the dev machine only stores the files).

Install the streamer and the libcamera V4L2 wrapper:

```bash
sudo apt update
sudo apt install -y ustreamer libcamera-tools libcamera-v4l2
```

If `ustreamer` is not packaged for your image, build it:

```bash
sudo apt install -y build-essential libevent-dev libjpeg62-turbo-dev libbsd-dev
git clone --depth=1 https://github.com/pikvm/ustreamer
cd ustreamer && make -j$(nproc) && sudo make install
```

Test it manually before installing the service:

```bash
libcamerify ustreamer \
  --host=127.0.0.1 --port=8081 \
  --resolution=640x480 --desired-fps=12 \
  --quality=80 --encoder=CPU --slowdown
curl -sI http://127.0.0.1:8081/stream
```

Install the service (adjust `User=`, or the `libcamerify`/`ustreamer` paths, if
they differ on your system):

```bash
sudo cp systemd/ustreamer.service /etc/systemd/system/
sudo systemctl daemon-reload
sudo systemctl enable --now ustreamer
systemctl status ustreamer
journalctl -u ustreamer -f   # tail logs
```

Reload Caddy with the updated `systemd/Caddyfile`, then verify the public URL:

```bash
curl -sI https://4l3a.duckdns.org/camera/stream
```

`ustreamer` stays bound to `127.0.0.1:8081`; only Caddy exposes it over TLS.
MJPEG bandwidth is high (roughly 4–10 Mbps per viewer at 640x480/12 fps), so
lower the resolution, FPS or quality in `systemd/ustreamer.service` if needed.

If `libcamerify` does not work with your camera stack, check that
`rpicam-hello --list-cameras` detects the camera; a fallback is to pipe
`rpicam-vid -t 0 --codec mjpeg ... -o -` into an MJPEG HTTP server.

## Where to add your custom action

Edit `onClientVisit()` in `src/main.cpp`. It currently logs to console and to
`visits.log`, and bumps an in-memory counter exposed at `/api/status`. Add
GPIO control, webhook calls, database writes, etc. there — see the comments
in that function for a few concrete starting points.

If your action takes any real time (network calls, GPIO with delays, etc.),
dispatch it to a `std::thread` or a small task queue instead of running it
inline, so it doesn't add latency to every page load.

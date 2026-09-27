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

## Where to add your custom action

Edit `onClientVisit()` in `src/main.cpp`. It currently logs to console and to
`visits.log`, and bumps an in-memory counter exposed at `/api/status`. Add
GPIO control, webhook calls, database writes, etc. there — see the comments
in that function for a few concrete starting points.

If your action takes any real time (network calls, GPIO with delays, etc.),
dispatch it to a `std::thread` or a small task queue instead of running it
inline, so it doesn't add latency to every page load.

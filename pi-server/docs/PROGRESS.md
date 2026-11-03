# Progress (update at end of every session)

## Current state
- Repository workflow is defined at the root (`AGENTS.md`, `.opencode/`);
  `pi-server/AGENTS.md` holds project-specific rules.
- `app_core`, `server`, and `app_tests` CMake targets build with C++20 and
  warnings-as-errors.
- GoogleTest/CTest is wired; 34 backend tests cover SessionManager,
  WordSanitizer, MazeCode, and MinitelController (with FakeMaze).
- Frontend helpers live in `public/session-utils.mjs`; 6 Node tests cover them.
- `src/minitel/Minitel1B_Hard.{h,cpp}` is treated as vendored: it is built into
  `minitel_platform` but excluded from project format/lint checks.
- `make check` runs format checks, clang-tidy, build, backend tests, frontend
  tests, and ASan/UBSan tests.
- Camera bridge (`scripts/camera_stream.py` + `systemd/camera-stream.service`)
  is operational on the Pi. On 2026-11-03 all three endpoints were verified
  read-only through Caddy at `https://4l3a.hazardous-editorial.net:30777`:
  `/camera/state` returned `"online": true` (640x480, 12 fps), `/camera/snapshot`
  returned a 35 KB 640x480 JPEG, and `/camera/stream` delivered ~46 frames
  (~1.6 MB) in a 4 s capture.
- The camera section of `README.md` now documents the `rpicam-vid` bridge and
  its three endpoints instead of `ustreamer`.

## In progress
- (none)

## Next steps (ordered; take the first one)
1. On the Pi, confirm `camera-stream.service` is enabled at boot and
   `ustreamer.service` is disabled (cannot be checked remotely).
2. Keep `systemd/Caddyfile` in sync with the deployed Caddyfile; the live host
   also serves `4l3a.hazardous-editorial.net`, which is absent from the copy in
   this repository.

## Known issues
- The camera bridge only implements `GET`; `HEAD` on any camera endpoint
  returns `501`, so health checks must use `GET`.

## Unverified details
- Exact `rpicam-apps` / `librpicam-app1` versions on the Pi, and whether the
  earlier `generateConfiguration` undefined-symbol error is fixed by a package
  upgrade or no longer applies. The bridge is producing frames, so
  `rpicam-vid` works in the current setup.
- Whether `camera-stream.service` is enabled at boot and `ustreamer.service` is
  disabled/masked; both are configured in the repo unit but their live state
  was not inspected (no SSH access used).
- Sensor model reported by `rpicam-hello --list-cameras`; measured throughput
  during verification was ~11.5 fps at 640x480.

## Maintenance notes
- `make check` discovers `clang-tidy` on `PATH`, then through Homebrew LLVM
  (`brew --prefix llvm`). If neither is available, install it or pass an
  explicit `CLANG_TIDY=/path/to/clang-tidy`; the gate fails with an explicit
  message instead of silently skipping the lint.

## How to build / run
- `make build`, `make test`, `make test-frontend`, `make check`

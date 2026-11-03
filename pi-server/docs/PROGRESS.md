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
- Camera bridge files exist (`scripts/camera_stream.py`,
  `systemd/camera-stream.service`); Pi camera deployment still needs
  verification.

## In progress
- (none)

## Next steps (ordered; take the first one)
1. On the Pi, upgrade `rpicam-apps-core` and `librpicam-app1` to `1.14.0-1`,
   verify `rpicam-vid`, then enable `camera-stream.service` and disable
   `ustreamer`.
2. Verify `/camera/stream`, `/camera/snapshot`, and `/camera/state` through
   Caddy.
3. Update the camera section of `README.md` to describe the bridge service
   instead of ustreamer.

## Known issues
- Pi `rpicam-vid` currently fails with a `libcamera::Camera::generateConfiguration`
  undefined-symbol error until the rpicam packages are upgraded.
- `make check` discovers `clang-tidy` on `PATH`, then through Homebrew LLVM
  (`brew --prefix llvm`). If neither is available, install it or pass an
  explicit `CLANG_TIDY=/path/to/clang-tidy`; the gate fails with an explicit
  message instead of silently skipping the lint.

## How to build / run
- `make build`, `make test`, `make test-frontend`, `make check`

# pi-server project rules

Repository-wide session protocol lives in `../AGENTS.md`; it is authoritative.
This file covers project-specific architecture, conventions and quality
requirements.

## Stack
- Backend: C++ (C++20), CMake, no heavy frameworks. Frontend: plain HTML + JavaScript (ES modules) + CSS. No frontend framework, no bundler.
- Layout (relative to `pi-server/`): `src/` (backend), `src/services/` (logic, no httplib types), `third_party/`, `inc/`, `tests/` (backend tests), `public/` (static files), `docs/`.
- HTTP server: cpp-httplib (vendored in `third_party/httplib/httplib.h`, never edit it). Frontend files are served from `public/`.
- Do not add dependencies without asking. Prefer the standard library.

## Quality gates
- From the repository root, `make check -C pi-server` = format check + lint + build (warnings as errors) + tests (+ sanitizers). It must pass before any commit.
- Every new behavior gets a test. Bug fix = a failing test first.
- Never disable a warning, test, or lint rule to make things pass. Ask instead.

## Code rules
- Follow the skills in `../.opencode/skills/`: `cpp-backend`, `cpp-httplib`, `testing`, `code-review`, `git-workflow`, `frontend-vanilla`, `web-security`. Load the relevant one before coding.
- Small functions, clear names, no commented-out code, no magic numbers.
- Never invent APIs. Check existing code or docs (context7) first. If unsure, ask.
- Commits: small, conventional (`feat:`, `fix:`, `refactor:`, `test:`, `docs:`, `chore:`).

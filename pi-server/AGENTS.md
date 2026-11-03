# Project rules

## Stack
- Backend: C++ (C++20), CMake, no heavy frameworks. Frontend: plain HTML + JavaScript (ES modules) + CSS. No frontend framework, no bundler.
- Layout: `src/` (backend), `src/services/` (logic, no httplib types), `third_party/`, `inc/`, `tests/` (backend tests), `public/` (static files), `docs/`.
- HTTP server: cpp-httplib (vendored in `third_party/httplib/httplib.h`, never edit it). Frontend files are served from `public/`.
- Do not add dependencies without asking. Prefer the standard library.

## Session protocol (MANDATORY)
1. Start: read `docs/SPEC.md` and `docs/PROGRESS.md`, run `git status` and `git log --oneline -10`.
2. Work on ONE task at a time, taken from "Next steps" in PROGRESS.md.
3. Finish: run `make check`. Fix every failure. Then update `docs/PROGRESS.md` and commit.

## Quality gates
- `make check` = format check + lint + build (warnings as errors) + tests (+ sanitizers). It must pass before any commit.
- Every new behavior gets a test. Bug fix = a failing test first.
- Never disable a warning, test, or lint rule to make things pass. Ask instead.

## Code rules
- Follow the skills: `cpp-backend`, `cpp-httplib`, `testing`, `code-review`, `git-workflow`, `frontend-vanilla`, `web-security`. Load the relevant one before coding.
- Small functions, clear names, no commented-out code, no magic numbers.
- Never invent APIs. Check existing code or docs (context7) first. If unsure, ask.
- Commits: small, conventional (`feat:`, `fix:`, `refactor:`, `test:`, `docs:`, `chore:`).

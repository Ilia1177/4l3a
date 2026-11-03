---
name: testing
description: How to write and run tests for the C++ backend and vanilla JS frontend. Use when adding features, fixing bugs, or when tests fail.
---
## Rules
- Backend tests: GoogleTest or Catch2 in `tests/`, run through CTest. One behavior per test, names like `Parser_RejectsEmptyBody`.
- Bug fix: write a failing test that reproduces it first, then fix.
- Test business logic directly (no network). Mock/stub only at boundaries (storage, clock, I/O).
- Cover edge cases: empty, max size, invalid UTF-8, concurrency where relevant.
- Frontend: keep logic in pure functions inside ES modules so it can be tested with `node --test` without a browser.
- Never delete or weaken a test to get green. If a test is wrong, explain why before changing it.
- Run: `make test` (fast) and `make check` (full).

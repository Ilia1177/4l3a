---
name: code-review
description: Self-review checklist to run before declaring a task done, and used by the reviewer agent.
---
Check each item against the diff:
1. Correctness: does it satisfy the SPEC requirement? Edge cases handled?
2. Tests: new behavior covered? Tests meaningful (not just calling the code)?
3. Memory/ownership: leaks, dangling refs, iterator invalidation, data races.
4. Errors: every failure path handled and reported; no ignored return values.
5. Security: input validation, injection, path traversal, unbounded sizes (see `web-security`).
6. Design: small functions, no duplication, layers respected, names clear.
7. Scope: no unrelated changes, no leftover debug code, no new dependency without approval.
8. Docs: PROGRESS.md / SPEC.md / DECISIONS.md updated if needed.
Report: blocking / important / nit, each with file:line and a concrete fix.

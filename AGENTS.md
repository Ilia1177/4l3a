# Repository instructions

This file is authoritative for the repository-wide workflow. Project-specific
architecture, conventions and quality requirements live in
`pi-server/AGENTS.md`; read it before changing anything under `pi-server/`.

## Layout
- `pi-server/` - C++20 backend and vanilla HTML/JS/CSS frontend.
- `pi-server/docs/` - spec, progress and decisions log.
- `.opencode/` - OpenCode commands, agents and skills, discovered from the
  repository root.

## Session protocol (MANDATORY)

### Start
1. Read `pi-server/docs/SPEC.md`, `pi-server/docs/PROGRESS.md` and
   `pi-server/docs/DECISIONS.md`. `SPEC.md` is a template: never invent
   requirements, ask the user if it is unfilled.
2. Read `pi-server/AGENTS.md` for project rules.
3. Load the relevant skill from `.opencode/skills/` before coding.
4. Run `git status` and `git log --oneline -10`.
5. Identify existing uncommitted changes. Never overwrite or commit unrelated
   user changes.

### Work
1. Work on ONE task at a time, selected from "Next steps" in
   `pi-server/docs/PROGRESS.md`.
2. Follow the `testing` and `code-review` skills.
3. Do not start another task until the current one is completed.

### Finish (MANDATORY)
A task is NOT complete until it has been committed.

1. From the repository root, run `make check -C pi-server`. clang-tidy is
   discovered on `PATH` or through Homebrew LLVM; override it with
   `make -C pi-server CLANG_TIDY=/path/to/clang-tidy check` if needed.
2. Fix all failures. Never bypass quality gates.
3. Perform a final code review.
4. Update `pi-server/docs/PROGRESS.md`.
5. Update `pi-server/docs/DECISIONS.md` if an architectural decision was made.
6. Run `git status` and inspect the complete diff.
7. Stage ONLY files belonging to the current task.
8. Create a conventional commit.
9. Verify the commit with `git log -1 --oneline`.
10. Verify the working tree with `git status`.

Never push.

If tests fail, do not commit.
If Git refuses to commit, explain the exact error.
Never report a task as completed without verifying its commit.

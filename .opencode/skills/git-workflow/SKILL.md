---
name: git-workflow
description: Commit and branch conventions. Use when committing, branching, or finishing a task.
---
- One logical change per commit; build and tests pass at every commit.
- Message: `type(scope): imperative summary` (<=72 chars). Types: feat, fix, refactor, test, docs, chore, perf, build. Sign commit with "AGENT".
- Body explains why, not what, when non-obvious.
- Branch per feature: `feat/<short-name>`, `fix/<short-name>`. Never commit directly to a broken main.
- Check `git status` before committing; never commit build outputs, secrets, or `.env` files.
- Never push or rewrite history unless the user asks.

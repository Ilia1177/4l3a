---
name: web-security
description: Security checklist for the C++ HTTP backend and browser frontend. Use when handling requests, files, auth, or user input.
---
- Treat every request field, header, path and body as hostile: enforce max sizes, types, ranges.
- Static files: normalize path, reject `..`, serve only from the frontend directory.
- Never build SQL or shell commands by string concatenation; use prepared statements / avoid `system()`.
- Passwords: salted slow hash (argon2/bcrypt), never plaintext. Secrets from environment, never in git.
- Set headers: `Content-Security-Policy`, `X-Content-Type-Options: nosniff`, correct `Content-Type`; restrict CORS to known origins.
- Don't leak internals in error messages; log details server-side only.
- Frontend: no `innerHTML`/`eval` with user data; escape on output.

---
name: cpp-backend
description: Modern C++20 backend coding standards (ownership, errors, RAII, concurrency, project layout). Use when writing or changing any C++ code.
---
## Rules
- C++20. No raw `new`/`delete`; use RAII, `std::unique_ptr`, containers. `shared_ptr` only with a justification.
- Pass by `const&` or `std::string_view`/`std::span` for read-only; return by value. Mark `const`, `noexcept`, `[[nodiscard]]` where right.
- No `using namespace std;` in headers. Headers self-contained, `#pragma once`.
- Errors: no silent failures. Use exceptions at boundaries or `std::expected`-style results consistently (follow what the project already uses).
- No global mutable state. Protect shared data (`std::mutex` + `std::scoped_lock`); prefer immutable data/message passing.
- Separate layers: routes (cpp-httplib handlers, see `cpp-httplib` skill) -> services (pure logic, no httplib types) -> storage. Logic must be testable without a network.
- Validate ALL external input (size, range, encoding) before use. No `strcpy`/`sprintf`/C arrays; use `std::string`, `std::array`.
- Use `enum class`, `std::optional`, `std::variant` instead of magic values.
- Compile clean with `-Wall -Wextra -Wpedantic -Werror`. Run under ASan/UBSan in `make check`.
## Before finishing
Run `make check`. Fix warnings, do not suppress them.

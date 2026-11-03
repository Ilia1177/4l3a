# Agentic coding kit (C++ backend + vanilla frontend)
1. Copy everything into your project root (merge, don't overwrite your CMakeLists.txt).
2. Put httplib.h in `third_party/httplib/httplib.h` (keep it out of src/ and include/ so it is not formatted or linted). In your root CMakeLists.txt: `include(cmake/Quality.cmake)`, `include(cmake/Dependencies.cmake)`, `enable_testing()`, and call `apply_quality(<target>)` on your targets. Add a test target (GoogleTest/Catch2) registered with `add_test`.
3. Install tools: cmake, clang-format, clang-tidy, node (only for Biome lint/format of frontend).
4. Fill `pi-server/docs/SPEC.md` and `pi-server/docs/PROGRESS.md`; adapt
   `AGENTS.md` and `pi-server/AGENTS.md`.
5. In opencode: `/resume` to start, `/checkpoint` to stop, `/review` after a task.
Note: OpenCode discovers commands, agents and skills from `.opencode/` at the
repository root. If they are not picked up, try the singular folder names
(`command`, `agent`, `skill`).

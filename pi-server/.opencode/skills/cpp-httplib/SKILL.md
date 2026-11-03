---
name: cpp-httplib
description: How to structure, secure and test the backend built on cpp-httplib (yhirose/cpp-httplib, httplib.h). Use when adding or changing routes, handlers, static file serving, server settings, or HTTP tests.
---
## Structure (keep this shape)
- `third_party/httplib/httplib.h` is vendored. NEVER edit it, never format or lint it. Include as a SYSTEM header.
- `src/server.cpp` exposes `void register_routes(httplib::Server&, Services&)`. `main.cpp` only creates services, the server, calls `register_routes`, and `listen`.
- Handlers are thin: parse + validate input -> call a service function -> serialize result. Business logic lives in `src/services/` with NO httplib types, so it is unit-testable.
- One file per route group (`routes_users.cpp`, ...). Shared helpers: `json_error(res, status, msg)`, `parse_json_body(req)`.

## Handlers
- Signature: `[&](const httplib::Request& req, httplib::Response& res) { ... }`. Always set status and `Content-Type` (`res.set_content(body, "application/json")`).
- Path params: `svr.Get("/api/items/:id", ...)` then `req.path_params.at("id")`. Query params: `req.has_param("x")`, `req.get_param_value("x")`.
- Convert strings to numbers with `std::from_chars` and check the result; never `std::stoi` without try/catch.
- Errors: return a consistent JSON `{"error": "..."}` with the right status (400 invalid input, 404 missing, 409 conflict, 500 internal). Never throw out of a handler; also set `svr.set_exception_handler(...)` to return a generic 500 and log details server-side.
- Handlers run on multiple threads: any shared state needs a mutex (`std::scoped_lock`) or must be immutable.

## Server settings (set explicitly in one `configure_server()` function)
- `svr.set_payload_max_length(...)` (reject oversized bodies), `set_read_timeout`, `set_write_timeout`, `set_keep_alive_max_count`.
- Thread pool size configured deliberately (task queue / `CPPHTTPLIB_THREAD_POOL_COUNT`); do not leave unbounded work inside handlers (no long blocking calls).
- Static frontend: `svr.set_mount_point("/", "./public")`. Never build file paths from request input yourself.
- Security headers via `svr.set_post_routing_handler(...)`: `X-Content-Type-Options: nosniff`, `Content-Security-Policy`, `Referrer-Policy`. CORS: allow only known origins, never `*` with credentials.
- Bind to `127.0.0.1` by default; use `0.0.0.0` only when needed and documented. HTTPS needs `CPPHTTPLIB_OPENSSL_SUPPORT` (usually put a reverse proxy in front instead).
- Logging: `svr.set_logger(...)` with method, path, status, duration. Never log secrets or full bodies.

## JSON
- httplib has no JSON. Use one JSON library consistently (ask before adding, e.g. nlohmann/json). Validate every field (presence, type, range, length) before use.

## Testing (required for every route)
- Build the server in a fixture, start it on a free port, call it with `httplib::Client`, stop it:
```cpp
class ApiTest : public ::testing::Test {
 protected:
  void SetUp() override {
    register_routes(svr_, services_);
    port_ = svr_.bind_to_any_port("127.0.0.1");
    thread_ = std::thread([this] { svr_.listen_after_bind(); });
    svr_.wait_until_ready();
  }
  void TearDown() override { svr_.stop(); thread_.join(); }
  httplib::Server svr_;
  Services services_;
  int port_{};
  std::thread thread_;
};
TEST_F(ApiTest, GetUnknownItem_Returns404) {
  httplib::Client cli("127.0.0.1", port_);
  auto res = cli.Get("/api/items/999");
  ASSERT_TRUE(res);
  EXPECT_EQ(res->status, 404);
}
```
- Test: success, invalid input, missing resource, oversized body, wrong method, path traversal attempt (`/../`).
- Unit-test services separately without any server.
- Check exact API names against the vendored `httplib.h` or Context7 before using one; never guess.

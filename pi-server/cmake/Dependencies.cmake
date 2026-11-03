# Include from root CMakeLists.txt AFTER project():  include(cmake/Dependencies.cmake)
include(FetchContent)

# GoogleTest for unit + integration tests
FetchContent_Declare(googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.14.0)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# cpp-httplib is vendored: third_party/httplib/httplib.h
add_library(httplib_header INTERFACE)
# SYSTEM => warnings from httplib.h do not break -Werror builds
target_include_directories(httplib_header SYSTEM INTERFACE ${CMAKE_SOURCE_DIR}/third_party/httplib)
find_package(Threads REQUIRED)
target_link_libraries(httplib_header INTERFACE Threads::Threads)

# Example wiring in your CMakeLists.txt:
#   add_library(app_core src/server.cpp src/services/...)       # everything except main.cpp
#   target_link_libraries(app_core PUBLIC httplib_header)
#   apply_quality(app_core)
#   add_executable(app src/main.cpp)  target_link_libraries(app PRIVATE app_core)
#   add_executable(app_tests tests/test_api.cpp tests/test_services.cpp)
#   target_link_libraries(app_tests PRIVATE app_core GTest::gtest_main)
#   include(GoogleTest)  gtest_discover_tests(app_tests)

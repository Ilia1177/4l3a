add_executable(app_tests
    MazeCodeTest.cpp
    MinitelControllerTest.cpp
    SessionManagerTest.cpp
    WordSanitizerTest.cpp
)
target_link_libraries(app_tests PRIVATE app_core GTest::gtest_main)
apply_quality(app_tests)

include(GoogleTest)
gtest_discover_tests(app_tests)

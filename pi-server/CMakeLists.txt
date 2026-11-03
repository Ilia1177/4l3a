cmake_minimum_required(VERSION 3.14)
project(pi_server CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

include(cmake/Quality.cmake)
include(cmake/Dependencies.cmake)

find_package(PNG REQUIRED)
find_package(Threads REQUIRED)
enable_testing()

add_library(minitel_platform STATIC
    src/minitel/Minitel1B_Hard.cpp
)
target_include_directories(minitel_platform PUBLIC ${CMAKE_SOURCE_DIR}/inc)
target_link_libraries(minitel_platform PUBLIC PNG::PNG Threads::Threads)

add_library(app_core STATIC
    src/MinitelController.cpp
    src/SessionManager.cpp
    src/routes.cpp
    src/gemma.cpp
    src/maze/Maze.cpp
    src/minitel/HardwareSim.cpp
    src/services/MazeCode.cpp
    src/services/WordSanitizer.cpp
    src/utils/utils.cpp
    src/utils/Logger.cpp
)
target_include_directories(app_core PUBLIC ${CMAKE_SOURCE_DIR}/inc)
target_link_libraries(app_core PUBLIC minitel_platform httplib::httplib Threads::Threads PNG::PNG)
apply_quality(app_core)

add_executable(server src/main.cpp)
target_link_libraries(server PRIVATE app_core)
apply_quality(server)

add_subdirectory(tests)

file(CREATE_LINK ${CMAKE_SOURCE_DIR}/public ${CMAKE_BINARY_DIR}/public SYMBOLIC)

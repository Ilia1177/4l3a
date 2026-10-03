cmake_minimum_required(VERSION 3.14)
project(pi_server CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Release by default (Pi appreciates the optimization + smaller binary)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

include(FetchContent)
FetchContent_Declare(
  httplib
  GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
  GIT_TAG v0.15.3
)
FetchContent_MakeAvailable(httplib)
find_package(PNG REQUIRED)
find_package(Threads REQUIRED)
add_executable(server
    src/main.cpp
    src/SessionManager.cpp
    src/routes.cpp
	src/maze/Maze.cpp
    src/minitel/HardwareSim.cpp
    src/minitel/Minitel1B_Hard.cpp
    src/utils/utils.cpp
	src/utils/Logger.cpp
)
target_include_directories(server PRIVATE
    ${CMAKE_SOURCE_DIR}/inc
)
target_link_libraries(server PRIVATE PNG::PNG httplib::httplib Threads::Threads)
# Copy the public/ folder next to the binary so relative paths just work
file(COPY ${CMAKE_SOURCE_DIR}/public DESTINATION ${CMAKE_BINARY_DIR})

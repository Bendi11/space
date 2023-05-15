cmake_minimum_required(VERSION 3.20)

set(
    CMAKE_CXX_STANDARD 20
    CMAKE_CXX_STANDARD_REQUIRED on
    CMAKE_CXX_EXTENSIONS off
)

project(space)

Include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.3.2
)

FetchContent_MakeAvailable(Catch2)

add_subdirectory("api/cxximpl")
add_subdirectory("server")

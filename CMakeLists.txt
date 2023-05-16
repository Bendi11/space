cmake_minimum_required(VERSION 3.20)

set(
    CMAKE_CXX_STANDARD 20
    CMAKE_CXX_STANDARD_REQUIRED on
    CMAKE_CXX_EXTENSIONS off
)

project(space)



add_subdirectory("api/cxximpl")
add_subdirectory("server")

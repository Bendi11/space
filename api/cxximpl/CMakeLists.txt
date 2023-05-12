cmake_minimum_required(VERSION 3.20)

project(
    sendycxx
    LANGUAGES CXX
)

set(
    SRC
    "src/lib.cpp"
)

add_library(
    ${PROJECT_NAME}
    STATIC
    ${SRC}
)

target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

cmake_minimum_required(VERSION 3.20)

project(
    server
    LANGUAGES CXX
)

set(
    SRC
    "src/main.cpp"
)

add_executable(
    ${PROJECT_NAME}
    ${SRC}
)

target_compile_definitions(${PROJECT_NAME} PUBLIC ASIO_HAS_CO_AWAIT)

target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(${PROJECT_NAME} PRIVATE sendy)

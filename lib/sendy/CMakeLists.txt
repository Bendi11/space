cmake_minimum_required(VERSION 3.20)

project(
    sendy
    LANGUAGES CXX
)

set(object "${PROJECT_NAME}_obj")
set(TESTS "${PROJECT_NAME}_tests")

set(
    SRC
    "src/lib.cpp"
    "src/encode.cpp"
)

add_library(
    ${object}
    OBJECT
    ${SRC}
)

target_include_directories(${object} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

add_executable(
    ${TESTS}
    "src/tests/encoding.cpp"
)


target_link_libraries(${TESTS} PRIVATE ${object} Catch2::Catch2WithMain)

add_library(
    ${PROJECT_NAME}
    STATIC
)

target_link_libraries(${PROJECT_NAME} PUBLIC ${object})

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(CTest)
include(Catch)
catch_discover_tests(${TESTS} WORKING_DIRECTORY "$<TARGET_PROPERTY:${TESTS},BINARY_DIR>")

add_custom_command(
    TARGET ${TESTS}
    COMMENT "Run tests"
    POST_BUILD 
    WORKING_DIRECTORY "$<TARGET_PROPERTY:${TESTS},BINARY_DIR>"
    COMMAND ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION> -R "^${}$" --rerun-failed --output-on-failure
)

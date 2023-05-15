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

set(TESTS "${PROJECT_NAME}_tests")


file(GLOB_RECURSE TEST_FILES
    "${PROJECT_SOURCE_DIR}/src/tests/*.cpp"
)

add_executable(
    ${TESTS}
    ${SRC}
    ${TEST_FILES}
)

target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(${PROJECT_NAME} PRIVATE Catch2::Catch2WithMain)

target_link_libraries(${TESTS} PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME},LINK_LIBRARIES>)
target_include_directories(${TESTS} PUBLIC $<TARGET_PROPERTY:${PROJECT_NAME},INCLUDE_DIRECTORIES>)

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

# Special thanks to: https://github.com/ttroy50/cmake-examples

cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

enable_testing()

add_subdirectory(test)

project(termsequel VERSION 1.2.0)

configure_file(termsequel.hpp.in termsequel.hpp)

set(
    SOURCES
    source/lexer/token.cpp
    source/lexer/lexer.cpp
    source/parser/parser.cpp
    source/parser/node.cpp
    source/analyzer/analyzer.cpp
    source/executor/executor.cpp
    source/system/linux/system_iouring.cpp
    source/evaluator/evaluator.cpp
    source/view/view.cpp
    source/definitions.cpp
)

set(EXECUTABLE main)

add_compile_definitions(DEBUG=1)

add_library(
        ${PROJECT_NAME}
        INTERFACE
)

add_executable(${EXECUTABLE} ${SOURCES} termsequel.cpp)

target_include_directories(
    ${EXECUTABLE}
    PUBLIC ${PROJECT_BINARY_DIR} # CMake will put the termsequel.hpp file here
    PRIVATE ${PROJECT_SOURCE_DIR}/include
)

if(MSVC)
    target_compile_options(
        ${EXECUTABLE}
        PRIVATE /W4 /WX
    )
    set_target_properties(
        ${EXECUTABLE}
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_BINARY_DIR}/executable
    )
else ()

    find_package(PkgConfig REQUIRED)
    pkg_check_modules(LIBURING REQUIRED liburing)
    include_directories(${LIBURING_INCLUDE_DIRS})
    target_link_libraries(${EXECUTABLE} ${LIBURING_LIBRARIES})

    target_compile_options(
        ${EXECUTABLE}
        PRIVATE -Wall -Werror -g -Wextra
    )
    set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}/executable)
endif()





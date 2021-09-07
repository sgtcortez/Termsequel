# Special thanks to: https://github.com/ttroy50/cmake-examples

cmake_minimum_required(VERSION 3.10)

project(termsequel VERSION 1.0.2)

configure_file(termsequel.hpp.in termsequel.hpp)

set(
    SOURCES
    src/system.cpp
    src/compiler.cpp
    termsequel.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC ${PROJECT_BINARY_DIR} # CMake will put the termsequel.hpp file here
    PRIVATE ${PROJECT_SOURCE_DIR}/include
)

if(MSVC)
    target_compile_options(
        ${PROJECT_NAME}
        PRIVATE /W4 /WX
    )
    set_target_properties(
        ${PROJECT_NAME}
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_BINARY_DIR}/executable
    )
else ()
    target_compile_options(
        ${PROJECT_NAME}
        PRIVATE -Wall -Werror -Wextra
    )
    set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}/executable)
endif()

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

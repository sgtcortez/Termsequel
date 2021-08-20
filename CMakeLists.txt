# Special thanks to: https://github.com/ttroy50/cmake-examples

cmake_minimum_required(VERSION 3.10)

project(termsequel VERSION 0.1)

set(
    SOURCES
    src/system.cpp
    src/compiler.cpp
    termsequel.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(
    ${PROJECT_NAME}
    PRIVATE ${PROJECT_SOURCE_DIR}/include
)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

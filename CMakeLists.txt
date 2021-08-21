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

if(MSVC)
    target_compile_options(
        ${PROJECT_NAME}
        PRIVATE /W4 /WX
    )
else ()
    target_compile_options(
        ${PROJECT_NAME}
        PRIVATE -Wall -Werror -Wextra
    )
endif()


set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

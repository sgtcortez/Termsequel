# Special thanks to: https://github.com/ttroy50/cmake-examples

cmake_minimum_required(VERSION 3.10)

project(termsequel VERSION 1.2.0)

include(ExternalProject)

ExternalProject_Add(
        cgeneric
        GIT_REPOSITORY https://github.com/sgtcortez/cgeneric.git
        GIT_TAG        master

        # Where to put things
        PREFIX         ${CMAKE_BINARY_DIR}/third_party/cgeneric

        # Build system is now CMake
        CONFIGURE_COMMAND cmake -S . -B build
        BUILD_COMMAND     cmake --build build
        INSTALL_COMMAND   ""         # no install

        # Optional: control parallelism
        BUILD_IN_SOURCE   1
)

set(
    SOURCES
    
        source/frontend/token.c
        source/frontend/lexer.c
        source/frontend/parser.c
        source/frontend/analyzer.c
        source/frontend/node.c
        source/frontend/expression_debug.c

        source/string/string.c
        source/log/logger.c


        source/backend/system_linux.c
        source/backend/system_bsd.c
        source/backend/evaluator.c

        source/backend/plan/logical_plan.c
        source/backend/plan/physical_plan.c
        source/backend/plan/physical_plan_filter.c        
        source/backend/plan/physical_plan_project.c        
        source/backend/plan/physical_plan_sort.c
        source/backend/plan/physical_plan_output.c        
        source/backend/plan/physical_plan_scan.c
)

add_executable(${PROJECT_NAME} ${SOURCES} termsequel.c)
add_dependencies(${PROJECT_NAME} cgeneric)

target_include_directories(
        ${PROJECT_NAME}
        PUBLIC ${PROJECT_BINARY_DIR}
        PRIVATE ${PROJECT_SOURCE_DIR}/include
        PRIVATE ${CMAKE_BINARY_DIR}/third_party/cgeneric/src/cgeneric/include
)

target_link_libraries(
        ${PROJECT_NAME}
        PRIVATE ${CMAKE_BINARY_DIR}/third_party/cgeneric/src/cgeneric/build/libcgeneric.a

)

target_compile_options(
        ${PROJECT_NAME}
        PRIVATE -Wall -O0 -g3 -Wextra
)
option(ENABLE_INSTALL "Enable installation of termsequel" OFF)

if(ENABLE_INSTALL)
    include(GNUInstallDirs)

    install(
        TARGETS ${PROJECT_NAME}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif()

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

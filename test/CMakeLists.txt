project(test)

# Download gtest binary
include(FetchContent)
FetchContent_Declare(
        gtest
        URL https://github.com/google/googletest/archive/release-1.11.0.tar.gz
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

# configure build of googletest
FetchContent_MakeAvailable(gtest)

include_directories(../include)
include_directories(${CMAKE_BINARY_DIR}/_deps/googletest-src/googletest/include)

set(EXTERNAL_OBJECTS
        ../source/lexer/token.cpp
        ../source/lexer/lexer.cpp
        ../source/evaluator/evaluator.cpp
        ../source/parser/node.cpp
        ../source/parser/parser.cpp
)


# Creates the test for the lexer
add_executable(lexertest lexer.cpp ${EXTERNAL_OBJECTS})
target_include_directories(lexertest PRIVATE ../include)
target_link_libraries(lexertest gtest_main)
# https://cmake.org/cmake/help/book/mastering-cmake/chapter/Testing%20With%20CMake%20and%20CTest.html
add_test(NAME lexertests COMMAND ${CMAKE_CURRENT_BINARY_DIR}/lexertest)

add_executable(evaluatortest evaluator.cpp ${EXTERNAL_OBJECTS})
target_include_directories(evaluatortest PRIVATE ../include)
target_link_libraries(evaluatortest gtest_main)
add_test(NAME evaluatortests COMMAND ${CMAKE_CURRENT_BINARY_DIR}/evaluatortest)

add_executable(nodetest node.cpp ${EXTERNAL_OBJECTS} )
target_include_directories(nodetest PRIVATE ../include)
target_link_libraries(nodetest gtest_main)
add_test(NAME nodetests COMMAND ${CMAKE_CURRENT_BINARY_DIR}/nodetest)

add_executable(parsertest parser.cpp ${EXTERNAL_OBJECTS})
target_include_directories(parsertest PRIVATE ../include)
target_link_libraries(parsertest gtest_main)
add_test(NAME parsertests COMMAND ${CMAKE_CURRENT_BINARY_DIR}/parsertest)
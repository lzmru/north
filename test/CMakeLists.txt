cmake_minimum_required(VERSION 3.8)

find_package(LLVM REQUIRED CONFIG)
find_package(Catch2 REQUIRED)

project(lexer)

include_directories(
        ../libnorth/include
        ${LLVM_INCLUDE_DIRS}
)

set(SOURCE_FILES Lexer.cpp)

add_executable(lexer ${SOURCE_FILES})

llvm_map_components_to_libnames(llvm_libs all)
target_link_libraries(lexer ${llvm_libs} libnorth Catch2::Catch2)

include(CTest)
include(Catch)
catch_discover_tests(lexer)



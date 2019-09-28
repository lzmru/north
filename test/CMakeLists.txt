cmake_minimum_required(VERSION 3.8)

project(tests)

find_package(LLVM REQUIRED CONFIG)
find_package(Catch2 REQUIRED)

include_directories(
        ../libnorth/include
        ${LLVM_INCLUDE_DIRS}
)

add_executable(tests Lexer.cpp Parser.cpp)

llvm_map_components_to_libnames(llvm_libs all)
target_link_libraries(tests ${llvm_libs} libnorth Catch2::Catch2)

include(CTest)
include(Catch)

catch_discover_tests(tests)



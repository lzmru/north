cmake_minimum_required(VERSION 3.8)
project(north)

set(CMAKE_CXX_STANDARD 17)

find_package(LLVM REQUIRED CONFIG)

include_directories(
        ./include
        ${LLVM_INCLUDE_DIRS}
)

set(SOURCE_FILES
        src/Main.cpp
        src/Grammar/Lexer.cpp
        src/Grammar/Parser.cpp
        src/Grammar/Token.cpp
        src/Diagnostic.cpp
        src/Trace.cpp
        src/AST/Dumper.cpp
        src/IR/IRBuilder.cpp
        src/Type/Module.cpp
        src/Type/Scope.cpp
        src/Type/TypeInference.cpp
        src/Type/Type.cpp)

add_executable(north ${SOURCE_FILES})

llvm_map_components_to_libnames(
        llvm_libs
        core
        executionengine
        analysis
        native
        bitwriter
        aarch64asmparser aarch64asmprinter aarch64codegen aarch64desc aarch64disassembler aarch64info aarch64utils all)
target_link_libraries(north ${llvm_libs})
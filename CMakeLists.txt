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
        src/Targets/IR/StmtBuilder.cpp
        src/Type/Module.cpp
        src/Type/Scope.cpp
        src/Type/TypeInference.cpp
        src/Type/Type.cpp
        src/Targets/IR/ExprBuilder.cpp
        src/Targets/IR/StmtBuilder.cpp
        src/Targets/IR/DeclBuilder.cpp
        src/Targets/C/DeclBuilder.cpp
        src/Targets/C/StmtBuilder.cpp
        src/Targets/C/ExprBuider.cpp
        include/Targets/CBuilder.h
        include/Targets/BuilderBase.h
        src/Targets/IR/Utils.cpp)

add_executable(north ${SOURCE_FILES})

llvm_map_components_to_libnames(llvm_libs all)
target_link_libraries(north -v ${llvm_libs})
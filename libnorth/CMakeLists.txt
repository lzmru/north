cmake_minimum_required(VERSION 3.8)
project(libnorth)

find_package(LLVM REQUIRED CONFIG)

include_directories(
        ./include
        ${LLVM_INCLUDE_DIRS}
)

set(SOURCE_FILES
        src/Grammar/Lexer.cpp
        src/Grammar/Parser.cpp
        src/Grammar/Token.cpp
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
        src/Targets/IR/Utils.cpp
        src/Diagnostic.cpp)

add_library(libnorth ${SOURCE_FILES})
set_target_properties(libnorth PROPERTIES PREFIX "")

llvm_map_components_to_libnames(llvm_libs all)
target_link_libraries(libnorth ${llvm_libs})
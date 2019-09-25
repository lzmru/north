cmake_minimum_required(VERSION 3.8)
project(northc)

find_package(LLVM REQUIRED CONFIG)

include_directories(
        ./include
        ../libnorth/include
        ${LLVM_INCLUDE_DIRS}
)

set(SOURCE_FILES
        src/Main.cpp
        src/Trace.cpp
        src/Dumper.cpp
        )

add_executable(northc ${SOURCE_FILES})

llvm_map_components_to_libnames(llvm_libs all)

target_link_libraries(northc ${llvm_libs} libnorth)
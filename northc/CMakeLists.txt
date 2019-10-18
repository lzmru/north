cmake_minimum_required(VERSION 3.15)
project(northc)

find_package(LLVM REQUIRED CONFIG)

include_directories(
        ./include
        ../libnorth/include
        ${LLVM_INCLUDE_DIRS}
)

file(GLOB_RECURSE INCLUDE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/include" *.h)
file(GLOB_RECURSE SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src" *.cpp)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/include" PREFIX "Headers" FILES ${INCLUDE_FILES})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/src" PREFIX "Sources" FILES ${SOURCE_FILES})

add_executable(northc ${INCLUDE_FILES} ${SOURCE_FILES})

llvm_map_components_to_libnames(llvm_libs all)

target_link_libraries(northc ${llvm_libs} libnorth)

cmake_minimum_required(VERSION 3.15)
project(libnorth)

find_package(LLVM REQUIRED CONFIG)

include_directories(./include ${LLVM_INCLUDE_DIRS})
        
file(GLOB_RECURSE INCLUDE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/include" *.h)
file(GLOB_RECURSE SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src" *.cpp)

#set_property(TARGET ./include PROPERTY FOLDER "Headers")
#source_group("Headers" FILES ${INCLUDE_FILES})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/include" PREFIX "Headers" FILES ${INCLUDE_FILES})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/src" PREFIX "Sources" FILES ${SOURCE_FILES})

add_library(libnorth ${INCLUDE_FILES} ${SOURCE_FILES})
set_target_properties(libnorth PROPERTIES PREFIX "")

llvm_map_components_to_libnames(llvm_libs all)
target_link_libraries(libnorth ${llvm_libs})

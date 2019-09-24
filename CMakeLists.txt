cmake_minimum_required(VERSION 3.8)
project(north)

set(CMAKE_CXX_STANDARD 17)

set(OUTPUT_DIR ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR})

add_subdirectory(libnorth)
add_subdirectory(northc)

if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(test)
endif()
cmake_minimum_required(VERSION 3.15)
project(north)

set(CMAKE_CXX_STANDARD 20)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(OUTPUT_DIR ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR})

add_subdirectory(libnorth)
add_subdirectory(northc)

# Tests
if(BUILD_TESTING)
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_DIR}/test)
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR}/test)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}/test)

  enable_testing()
  add_subdirectory(test)
endif()
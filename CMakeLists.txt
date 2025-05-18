cmake_minimum_required(VERSION 3.22)

project(
    AABBlocks
    VERSION 1.0
    LANGUAGES CXX
)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

add_subdirectory(res)
add_subdirectory(vendor EXCLUDE_FROM_ALL)
add_subdirectory(src)
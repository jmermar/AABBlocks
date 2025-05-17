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


file(GLOB_RECURSE SRC
     "src/*.cpp"
)

add_executable (${PROJECT_NAME} ${SRC})

add_dependencies(${PROJECT_NAME} Shaders)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)

if(Win32)
    target_compile_options(${PROJECT_NAME} PRIVATE "/warnaserror")
elseif(UNIX)
    target_compile_options(${PROJECT_NAME} PRIVATE "-Wall" "-Werror")
endif(Win32)

target_compile_definitions(${PROJECT_NAME} PUBLIC GLM_FORCE_DEPTH_ZERO_TO_ONE)
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")


find_package(Vulkan REQUIRED)

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE
    ${Vulkan_LIBRARIES}
)

add_custom_command(TARGET AABBlocks POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/res"
        "${CMAKE_CURRENT_BINARY_DIR}/res"
    COMMENT "Coping reources to build directory"
)
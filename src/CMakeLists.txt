file(GLOB_RECURSE SRC
     "*.cpp"
     "utils/*.cpp"
     "rendering/*.cpp"
     "vk/*.cpp"
     "platform/linux/*.cpp"
)

add_executable (${PROJECT_NAME} ${SRC}
    utils/files.hpp
    utils/files.cpp)

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
find_package(OpenMP REQUIRED)
find_package(Threads REQUIRED)

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE
    ${Vulkan_LIBRARIES}
    SDL3::SDL3
    glm
    vma_static
    spdlog::spdlog
    vk-bootstrap::vk-bootstrap
    stb_image
    imgui
    OpenMP::OpenMP_CXX
    Threads::Threads
)

target_include_directories(${CMAKE_PROJECT_NAME}
  PRIVATE
    ${vma_SOURCE_DIR}/include
)

add_custom_command(TARGET AABBlocks POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/res"
        "${CMAKE_CURRENT_BINARY_DIR}/res"
    COMMENT "Coping reources to build directory"
)

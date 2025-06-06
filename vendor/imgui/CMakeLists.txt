project (imgui)

file(GLOB IMGUI_SRC
    "src/*.cpp"
    "src/backends/imgui_impl_sdl3.cpp"
    "src/backends/imgui_impl_vulkan.cpp"
)

add_library(imgui 
    ${IMGUI_SRC}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
SDL3::SDL3)

target_include_directories( ${PROJECT_NAME}
    PUBLIC ${PROJECT_SOURCE_DIR}/src
)
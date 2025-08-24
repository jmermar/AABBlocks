include(FetchContent)

FetchContent_Declare(
  SDL3
  GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
  GIT_TAG release-3.2.20
)
FetchContent_MakeAvailable(SDL3)

FetchContent_Declare(
  glm
  GIT_REPOSITORY https://github.com/g-truc/glm.git
  GIT_TAG 0.9.9.8
)
FetchContent_MakeAvailable(glm)

FetchContent_Declare(
  vkb
  GIT_REPOSITORY https://github.com/charles-lunarg/vk-bootstrap.git
  GIT_TAG v1.4.325
)
FetchContent_MakeAvailable(vkb)

FetchContent_Declare(
  sqlite
  GIT_REPOSITORY https://github.com/SRombauts/SQLiteCpp.git
  GIT_TAG 3.3.3
)
FetchContent_MakeAvailable(sqlite)

add_subdirectory(vma)
add_subdirectory(spdlog)
add_subdirectory(stb_image)
add_subdirectory(imgui)
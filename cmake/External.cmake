include(FetchContent)
  
## RENDERER
FetchContent_Declare(
  renderer  
  GIT_REPOSITORY https://github.com/Smutekj/simple-emscripten-renderer
  GIT_TAG master
)
FetchContent_MakeAvailable(renderer)

FetchContent_Declare(
  ecs  
  GIT_REPOSITORY https://github.com/Smutekj/ECS
  GIT_TAG main
)
FetchContent_MakeAvailable(ecs)


FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz)
FetchContent_MakeAvailable(json)


include(FetchContent)
  
## RENDERER
FetchContent_Declare(
  renderer  
  GIT_REPOSITORY https://github.com/Smutekj/simple-emscripten-renderer
  GIT_TAG master
)
FetchContent_MakeAvailable(renderer)

## tinyXML
FetchContent_Declare(
  tinyXML  
  GIT_REPOSITORY https://github.com/leethomason/tinyxml2
  GIT_TAG master
)
FetchContent_MakeAvailable(tinyXML)


FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz)
FetchContent_MakeAvailable(json)


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

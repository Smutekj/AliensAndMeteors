include(FetchContent)
    
 
## RENDERER
FetchContent_Declare(
  renderer  
  GIT_REPOSITORY https://github.com/Smutekj/simple-emscripten-renderer
  GIT_TAG VAO
)
FetchContent_MakeAvailable(renderer)

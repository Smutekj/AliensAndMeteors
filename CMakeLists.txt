cmake_minimum_required(VERSION 3.15)

# if( ${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
# project(project-web)
# else()
project(projectx)
# endif()

set(CMAKE_CXX_STANDARD 20)

include(cmake/External.cmake)
include(cmake/CompilerFlags.cmake)

file(GLOB SRC
     "src/*.h"
     "src/*.cpp"
     "src/Utils/*.h"
     "src/Utils/*.cpp"
     "src/Menu/*.h"
     "src/Menu/*.cpp"
     "src/Entities/*.h"
     "src/Entities/*.cpp"
)

include_directories(SYSTEM external/stbimage)
include_directories(SYSTEM external/magic_enum)
include_directories(SYSTEM external)
# include_directories(SYSTEM external/freetype/include)
message(${PROJECT_BINARY_DIR})
include_directories("${PROJECT_BINARY_DIR}" ${IMGUI_DIR} ${IMGUI_DIR}/backends )
add_executable(${CMAKE_PROJECT_NAME} ${SRC} )
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE 
                                             SDL2::SDL2main
                                            renderer )
                                             
set_target_compiler_flags(${CMAKE_PROJECT_NAME})

if( ${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")


     set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --profiling")

     #    embed resources and set target to .thml
     set(CMAKE_EXECUTABLE_SUFFIX .html)
     target_link_options(${CMAKE_PROJECT_NAME} PRIVATE
     "-sUSE_SDL=2"
     "-sFULL_ES3=1"
     "-sWASM=1"
     "-sUSE_WEBGL2=1"
     "-sOFFSCREEN_FRAMEBUFFER=1"
     "-sMIN_WEBGL_VERSION=2"
     "-sMAX_WEBGL_VERSION=2"
     "-sASSERTIONS=1"
     "-sALLOW_MEMORY_GROWTH=1"
     "-s ASYNCIFY=1"
     "-sEXCEPTION_CATCHING_ALLOWED=yes" 
     )
     target_link_options(${CMAKE_PROJECT_NAME} PRIVATE
     --embed-file=${CMAKE_CURRENT_SOURCE_DIR}/Resources/Fonts@../Resources/Fonts 
     --embed-file=${CMAKE_CURRENT_SOURCE_DIR}/external/lygia@../external/lygia
     --embed-file=${CMAKE_CURRENT_SOURCE_DIR}/Resources/Textures@../Resources/Textures
     --embed-file=${CMAKE_CURRENT_SOURCE_DIR}/Resources/Shaders@../Resources/Shaders
     )

endif()




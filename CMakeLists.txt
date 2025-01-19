cmake_minimum_required(VERSION 3.15)

if( ${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
project(project-web)
else()
project(projectx)
endif()

set(CMAKE_CXX_STANDARD 20)

if( ${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
     include(cmake/EmscriptenFlags.cmake)
endif()

include(cmake/External.cmake)

include(cmake/CompilerFlags.cmake)

message("\n\n LINKER AFTER \n\n ${COMPILE_OPTIONS}")

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
include_directories(SYSTEM external/lua/src)
include_directories(SYSTEM external/freetype/include)
include_directories("${PROJECT_BINARY_DIR}" ${IMGUI_DIR} ${IMGUI_DIR}/backends )
add_executable(${CMAKE_PROJECT_NAME} ${SRC} )
target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC 
                                            renderer
                                            freetype) 
set_compiler_flags(${CMAKE_PROJECT_NAME})



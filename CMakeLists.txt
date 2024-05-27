cmake_minimum_required(VERSION 3.22)
project(Boids)

find_package(Git)
message(STATUS "GIT_EXECUTABLE='${GIT_EXECUTABLE}'")

### CMake config ###
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) 
set(CMAKE_CXX_STANDARD 20)


if(NOT CMAKE_BUILD_TYPE) 
    set(CMAKE_BUILD_TYPE "Release")
endif()

set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS YES)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}")

add_library(project_warnings INTERFACE)
include(cmake/CompilerWarnings.cmake)
set_project_warnings(project_warnings)

# Link this 'library' to use the warnings specified in CompilerWarnings.cmake
include(cmake/CompilerFlags.cmake)
include(cmake/ExternalTools.cmake)

# project(lua LANGUAGES C)

# add_library(lua STATIC lua/onelua.c)

# target_include_directories(lua
#                            PUBLIC
#                            $<INSTALL_INTERFACE:lua>
#                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../lua>
#                            )
# target_compile_definitions(lua PRIVATE MAKE_LIB)
# # add_subdirectory(lua)

add_subdirectory(src)

include(FetchContent)


### Dependency declaration ###
FetchContent_Declare(
  sfml
  URL https://github.com/SFML/SFML/archive/refs/tags/2.5.1.zip
  URL_MD5 2c4438b3e5b2d81a6e626ecf72bf75be
)

### Dependency population ###
# sfml
set(BUILD_SHARED_LIBS OFF)
set(SFML_BUILD_EXAMPLES OFF)
set(SFML_BUILD_DOC OFF)
set(SFML_BUILD_NETWORK OFF)
set(SFML_BUILD_AUDIO ON)
set(SFML_BUILD_GRAPHICS OFF)
set(SFML_BUILD_WINDOW ON)

FetchContent_GetProperties(sfml)
if(NOT sfml_POPULATED)
  FetchContent_Populate(sfml)
  add_subdirectory(${sfml_SOURCE_DIR})
endif()

# include_directories(${SFML_INCLUDE_DIRS})
# include_directories(SYSTEM ${sfml_SOURCE_DIR}/include)
# # FetchContent_Declare(
# #         imgui
# #         GIT_REPOSITORY https://github.com/glfw/glfw
# # )

# # FetchContent_GetProperties(imgui)
# # if(NOT glfw_POPULATED)
# #     FetchContent_Populate(glfw)

# #     set(GLFW_BUILD_EXAMPLES OFF CACHE INTERNAL "Build the GLFW example programs")
# #     set(GLFW_BUILD_TESTS OFF CACHE INTERNAL "Build the GLFW test programs")
# #     set(GLFW_BUILD_DOCS OFF CACHE INTERNAL "Build the GLFW documentation")
# #     set(GLFW_INSTALL OFF CACHE INTERNAL "Generate installation target")

# #     add_subdirectory(${glfw_SOURCE_DIR} ${glfw_BINARY_DIR})
# # endif()


# # Dear ImGui
# FetchContent_Declare(
#   imgui
#   GIT_REPOSITORY https://github.com/ocornut/imgui
#   GIT_TAG 35b1148efb839381b84de9290d9caf0b66ad7d03 # 1.82
# )

# FetchContent_MakeAvailable(imgui)

# # ImGui-SFML
# FetchContent_Declare(
#   imgui-sfml
#   GIT_REPOSITORY https://github.com/SFML/imgui-sfml
#   GIT_TAG 82dc2033e51b8323857c3ae1cf1f458b3a933c35 # 2.3
# )
# FetchContent_MakeAvailable(imgui-sfml)
# # add_subdirectory(imgui-sfml)



# Dear ImGui
FetchContent_Declare(
  imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui
  GIT_TAG 35b1148efb839381b84de9290d9caf0b66ad7d03 # 1.82
)

FetchContent_MakeAvailable(imgui)

# ImGui-SFML
FetchContent_Declare(
  imgui-sfml
  GIT_REPOSITORY https://github.com/SFML/imgui-sfml
  GIT_TAG 82dc2033e51b8323857c3ae1cf1f458b3a933c35 # 2.3
)
set(IMGUI_DIR ${imgui_SOURCE_DIR})
set(IMGUI_SFML_FIND_SFML OFF)
set(IMGUI_SFML_IMGUI_DEMO ON)

FetchContent_MakeAvailable(imgui-sfml)
add_definitions(-D_USE_MATH_DEFINES)

# ========== place libraries and executables in outermost dir ==========

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})


# ========== cpp and header files ==========

set(hdrs   UI.h BVH.h core.h Commands.h   Application.h Particles.h
   Bloom.h Polygon.h )
set(objs UI.cpp BVH.cpp  Polygon.cpp  Commands.cpp Application.cpp Particles.cpp
    Bloom.cpp )

###### GAME ###### 
set(game_objs  BehaviourBase.cpp Behaviours.cpp Player.cpp
   Game.cpp SoundModule.cpp GameObject.cpp GameWorld.cpp Entities.cpp Animation.cpp)
set(game_hdrs ResourceManager.h BehaviourBase.h Behaviours.h Player.h
   Game.h  SoundModule.h GameObject.h GameWorld.h Entities.h Animation.h)

###### COLLISIONS #####
set(coll_objs CollisionSystem.cpp GridNeighbourSearcher.h )
set(coll_hdrs CollisionSystem.h GridNeighbourSearcher.cpp )


###### MENU #####
set(menu_objs  ScoreBoard.cpp Menu.cpp SettingsState.cpp StateStack.cpp State.cpp PauseState.cpp MenuState.cpp GameState.cpp)
set(menu_hdrs  ScoreBoard.h   Menu.h   SettingsState.h   StateStack.h   State.h   PauseState.h   MenuState.h   GameState.h)
list(TRANSFORM menu_objs PREPEND ${CMAKE_CURRENT_SOURCE_DIR}/Menu/ )
list(TRANSFORM menu_hdrs PREPEND ${CMAKE_CURRENT_SOURCE_DIR}/Menu/ )

set(utils_objs  Grid.h ObjectPool.h  RandomTools.h ) 
set(utils_hdrs Grid.cpp ObjectPool.cpp  ) 
list(TRANSFORM utils_objs PREPEND ${CMAKE_CURRENT_SOURCE_DIR}/Utils/ )
list(TRANSFORM utils_hdrs PREPEND ${CMAKE_CURRENT_SOURCE_DIR}/Utils/ )

add_executable(Boids main.cpp
         ${utils_objs} ${utils_hdrs} ${coll_hdrs} ${coll_objs}
          ${menu_objs} ${menu_hdrs}
        ${objs}  ${hdrs}  ${game_objs} ${game_hdrs} ${gui_objs} ${gui_hdrs}   )


#### THIS IS FOR GOOGLE TEST
# enable_testing()

target_link_libraries(Boids PRIVATE sfml-graphics sfml-audio sfml-window sfml-system ImGui-SFML::ImGui-SFML)


if(WIN32)
if (BUILD_SHARED_LIBS)
    add_custom_command(TARGET example_exe POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:ImGui-SFML::ImGui-SFML>
        $<TARGET_FILE:sfml-graphics>
        $<TARGET_FILE:sfml-window>
        $<TARGET_FILE:sfml-system>
        $<TARGET_FILE_DIR:example_exe>)
endif()
endif()
# include(GoogleTest)
# gtest_discover_tests(test_all)


add_definitions(-D_USE_MATH_DEFINES)

# ========== place libraries and executables in outermost dir ==========

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})


# ========== cpp and header files ==========
set(hdrs   UI.h  Polygon.h Commands.h   Application.h Particles.h Bloom.h )
set(objs UI.cpp Polygon.cpp  Commands.cpp Application.cpp Particles.cpp Bloom.cpp )

###### GAME ###### 
set(game_objs  BehaviourBase.cpp 
   Game.cpp SoundModule.cpp GameObject.cpp GameWorld.cpp Animation.cpp ObjectiveSystem.cpp)
set(game_hdrs ResourceManager.h BehaviourBase.h 
   Game.h  SoundModule.h GameObject.h GameWorld.h  Animation.h ObjectiveSystem.h)

##### ENTITIES #####
set(ent_objs Entities.cpp Player.cpp Enemy.cpp Attacks.h VisualEffects.h Triggers.h)
set(ent_hdrs Entities.h Player.h Enemy.h Attacks.cpp VisualEffects.cpp Triggers.cpp)
list(TRANSFORM ent_objs PREPEND ${CMAKE_CURRENT_SOURCE_DIR}/Entities/)
list(TRANSFORM ent_hdrs PREPEND ${CMAKE_CURRENT_SOURCE_DIR}/Entities/)


###### COLLISIONS #####
set(coll_objs AABB.h BVH.h CollisionSystem.cpp GridNeighbourSearcher.h )
set(coll_hdrs AABB.cpp BVH.cpp CollisionSystem.h GridNeighbourSearcher.cpp )


###### MENU #####
set(menu_objs  ScoreBoard.cpp Menu.cpp SettingsState.cpp StateStack.cpp State.cpp PauseState.cpp MenuState.cpp GameState.cpp)
set(menu_hdrs  ScoreBoard.h   Menu.h   SettingsState.h   StateStack.h   State.h   PauseState.h   MenuState.h   GameState.h)
list(TRANSFORM menu_objs PREPEND ${CMAKE_CURRENT_SOURCE_DIR}/Menu/ )
list(TRANSFORM menu_hdrs PREPEND ${CMAKE_CURRENT_SOURCE_DIR}/Menu/ )

set(utils_objs  Grid.h ObjectPool.h  RandomTools.h ) 
set(utils_hdrs Grid.cpp ObjectPool.cpp  ) 
list(TRANSFORM utils_objs PREPEND ${CMAKE_CURRENT_SOURCE_DIR}/Utils/ )
list(TRANSFORM utils_hdrs PREPEND ${CMAKE_CURRENT_SOURCE_DIR}/Utils/ )

add_executable(Boids main.cpp ${ent_objs} ${ent_hdrs}
         ${utils_objs} ${utils_hdrs} ${coll_hdrs} ${coll_objs}
          ${menu_objs} ${menu_hdrs}
        ${objs}  ${hdrs}  ${game_objs} ${game_hdrs} ${gui_objs} ${gui_hdrs}   )


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



#### Tests
set(TESTING true)
if(TESTING)
enable_testing()
include(GoogleTest)

add_executable(Tests Tests/main.cpp ${ent_objs} ${ent_hdrs}
        ${utils_objs} ${utils_hdrs} ${coll_hdrs} ${coll_objs}
         ${menu_objs} ${menu_hdrs}
       ${objs}  ${hdrs}  ${game_objs} ${game_hdrs} ${gui_objs} ${gui_hdrs}   )

target_link_libraries(Tests PRIVATE gtest sfml-graphics sfml-system sfml-window sfml-audio ImGui-SFML::ImGui-SFML)
gtest_discover_tests(Tests)
endif()

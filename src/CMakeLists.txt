add_definitions(-D_USE_MATH_DEFINES)

# ========== place libraries and executables in outermost dir ==========

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})


# ========== cpp and header files ==========

set(objs Menu.cpp SettingsState.cpp StateStack.cpp State.cpp PauseState.cpp MenuState.cpp GameState.cpp Application.cpp Particles.cpp
 ExplosionEffect.cpp UI.cpp ShaderManager.cpp BVH.cpp Selection.cpp  PolygonObstacleManager.h Polygon.h )
set(hdrs Commands.h Menu.h SettingsState.h StateStack.h State.h PauseState.h MenuState.h GameState.h Application.h Particles.h ExplosionEffect.h
 UI.h ShaderManager.h BVH.h core.h Selection.h PolygonObstacleManager.cpp Polygon.cpp)

###### GAME ###### 
set(game_objs BehaviourBase.h Behaviours.h Player.h BulletSystem.h BoidSystem.cpp Game.cpp SoundModule.cpp )
set(game_hdrs Behaviours.cpp Player.cpp BulletSystem.cpp BoidSystem.h Game.h  SoundModule.h )

set(utils_objs  Grid.h GayVector.h  RandomTools.h ) 
set(utils_hdrs Grid.cpp GayVector.cpp  ) 
list(TRANSFORM utils_objs PREPEND ${CMAKE_CURRENT_SOURCE_DIR}/Utils/ )
list(TRANSFORM utils_hdrs PREPEND ${CMAKE_CURRENT_SOURCE_DIR}/Utils/ )

add_executable(Boids main.cpp ${utils_hdrs} ${utils_objs}
${objs} ${game_objs} ${gui_objs} ${hdrs} ${game_hdrs} ${gui_hdrs}   )


#### THIS IS FOR GOOGLE TEST
# enable_testing()

target_link_libraries(Boids PRIVATE sfml-graphics sfml-window sfml-system ImGui-SFML::ImGui-SFML)


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


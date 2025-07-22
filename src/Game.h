#ifndef BOIDS_GAME_H
#define BOIDS_GAME_H

#include "Commands.h"
#include "GameWorld.h"
#include "DrawLayer.h"
#include "ObjectiveSystem.h"
#include "Camera.h"

class GameWorld;

class BossFight : public GameObject
{

public:
  int phase_id = 0;
};

class Game
{

public:
  enum class GameState
  {
    RUNNING,
    WON,
    PLAYER_DIED,
    SHOPPING
  };

  enum class GameStage
  {
    FREE,
    TIME_RACE,
    DODGE,
    ARENA,
  };

  GameStage stage = GameStage::FREE;

  void changeStage(GameStage to);

  Game(Renderer &window, KeyBindings &bindings);

  void update(const float dt, Renderer &win);
  void handleEvent(const SDL_Event &event);
  void parseInput(Renderer &window, float dt);
  void draw(Renderer &window);

  PlayerEntity *getPlayer();

  static bool isKeyPressed(SDL_Keycode key)
  {
    auto *keystate = SDL_GetKeyboardState(NULL);
    return keystate[SDL_GetScancodeFromKey(key)];
  }

  int getScore() const;
  GameState getState() const;

private:
  enum class ViewMoveState
  {
    FOLLOWING_PLAYER,
    FIXED,
    MOVING_TO_POSITION,
  };

  Camera m_camera;

  void drawUI(Renderer &window);
  void initializeLayers();

  void spawnNextObjective();
  void spawnBossObjective();
  void addDestroyNObjective(ObjectType type, int count);

  ObjectiveSystem m_objective_system;

  int m_score = 0;

  GameState m_state = GameState::RUNNING;

  Renderer &m_window;

  KeyBindings &m_key_binding;

  PlayerEntity *m_player;

  FrameBuffer m_scene_pixels;
  Renderer m_scene_canvas;

  View m_default_view;

  std::unique_ptr<GameWorld> m_world;

  std::unique_ptr<Font> m_font;
  Text m_health_text;

  TextureHolder m_textures;
  Texture m_background;
  LayersHolder m_layers;
  LayersHolder m_ui_layers;

  // friend UI;
  // UI m_ui;
};

#endif // BOIDS_GAME_H
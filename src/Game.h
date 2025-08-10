#ifndef BOIDS_GAME_H
#define BOIDS_GAME_H

#include "Commands.h"
#include "GameWorld.h"
#include "DrawLayer.h"
#include "ObjectiveSystem.h"
#include "UISystem.h"
#include "Camera.h"

#include "PostOffice.h"
#include "PostBox.h"

class GameWorld;

class BossFight : public GameObject
{

public:
  int phase_id = 0;
};

class Game
{

  PostOffice messanger;

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
    Free,
    TimeRace,
    Dodge,
    Arena,
    BossFight,
  };

  Game(Renderer &window, KeyBindings &bindings);
  ~Game()
  {
    std::cout << "HELLO FROM Game destructor!" << std::endl;
  }

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

  void changeStage(GameStage to);
  GameState getState() const;
  
  void initializeLayersAndTextures();
  void registerCollisions();

  void spawnNextObjective();
  void spawnBossObjective();
  void addDestroyNObjective(ObjectType type, int count);
  void startBossFight();

  GameStage m_stage = GameStage::Free;
  Camera m_camera;

  std::unique_ptr<ObjectiveSystem> m_objective_system;
  
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
  std::unique_ptr<Texture> m_background;
  LayersHolder m_layers;
  LayersHolder m_ui_layers;
  
  std::unique_ptr<UISystem> m_ui_system;

  std::unique_ptr<PostBox<EntityDiedEvent>> m_player_died_postbox;
  // friend UI;
};

#endif // BOIDS_GAME_H
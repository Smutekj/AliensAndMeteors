#pragma once

#include "Commands.h"
#include "GameWorld.h"
#include "DrawLayer.h"
#include "ObjectiveSystem.h"
#include "UISystem.h"
#include "Camera.h"

#include "PostOffice.h"
#include "PostBox.h"
#include "Entities/Factories.h"
#include "Systems/TimedEventManager.h"

#include "ToolBoxUI.h"
#include "QuestFactory.h"

class GameWorld;

class Spawner
{
public:
  Spawner(std::function<utils::Vector2f()> pos_generator) : m_pos_generator(pos_generator) {}

  virtual ~Spawner() = default;
  virtual void update(float dt) = 0;

protected:
  TimedEventManager m_events;
  std::function<utils::Vector2f()> m_pos_generator;
};

class EnemySpawner : public Spawner
{

public:
  EnemySpawner(GameWorld &world, TextureHolder &textures, EnemyType type,
               EnemySpec spec, std::function<utils::Vector2f()> pos_generator, float interval)
      : Spawner(pos_generator), m_type(type), m_spawner(world, textures)
  {
    m_events.addInfiniteEvent(interval, [this, spec, pos_generator](float t, int c)
                              {
                                auto &obj = m_spawner.create2(m_type, pos_generator(), spec);
                                m_spawned_ids.push_back(obj.getId());
                              });
  }

  virtual ~EnemySpawner() override = default;
  EnemySpawner(const EnemySpawner &other) = default;
  EnemySpawner(EnemySpawner &&other) = default;
  EnemySpawner &operator=(const EnemySpawner &other) = default;
  EnemySpawner &operator=(EnemySpawner &&other) = default;

  virtual void update(float dt) override
  {
    m_events.update(dt);
  };

private:
  EnemyType m_type;
  std::vector<std::size_t> m_spawned_ids;
  EnemyFactory2 m_spawner;
};

class PickupSpawner : public Spawner
{

public:
  PickupSpawner(GameWorld &world, TextureHolder &textures, Pickup type,
                std::function<utils::Vector2f()> pos_generator, float interval)
      : Spawner(pos_generator), m_type(type), m_spawner(world, textures) 
  {
    m_events.addInfiniteEvent(interval, [this, pos_generator](float t, int c)
                              { m_spawner.create2(m_type, pos_generator()); });
  }

  virtual ~PickupSpawner() override = default;
  PickupSpawner(const PickupSpawner &other) = default;
  PickupSpawner(PickupSpawner &&other) = default;
  PickupSpawner &operator=(const PickupSpawner &other) = default;
  PickupSpawner &operator=(PickupSpawner &&other) = default;

  virtual void update(float dt) override
  {
    m_events.update(dt);
  };

private:
  Pickup m_type;

  PickupFactory m_spawner;
};
class MeteorSpawner : public Spawner
{

public:
MeteorSpawner(GameWorld &world, TextureHolder &textures, MeteorType type,
                std::function<utils::Vector2f()> pos_generator, float interval)
      : Spawner(pos_generator), m_type(type), m_spawner(world, textures) 
  {
    m_events.addInfiniteEvent(interval, [this, pos_generator](float t, int c)
                              { m_spawner.create2(m_type, pos_generator()); });
  }

  virtual ~MeteorSpawner() override = default;

  virtual void update(float dt) override
  {
    m_events.update(dt);
  };

private:
  MeteorType m_type;

  MeteorFactory m_spawner;
};

class GameLevel
{

public:
  GameLevel(GameWorld &world, TextureHolder &textures) {}

  void update(float dt)
  {
    for (auto &spawner : m_spawners)
    {
      spawner->update(dt);
    }
  }

  // private:

  std::vector<std::shared_ptr<Spawner>> m_spawners;
  std::function<void()> m_on_stage_start = []() {};
  std::function<void()> m_on_stage_end = []() {};
};

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
  void initializeSounds();
  void loadTextures();
  void registerCollisions();
  void registerSystems();

  GameObject &createQuestGiver(std::shared_ptr<Quest> quest);

  void spawnNextObjective();
  void spawnBossObjective();
  void addDestroyNObjective(ObjectType type, int count);
  void startBossFight();
  void startSurvival();
  void startTimeRace();
  void startTimer();

  float m_timerace_timer;

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

  std::unique_ptr<EnemyFactory> m_enemy_factory;
  std::unique_ptr<PickupFactory> m_pickup_factory;
  std::unique_ptr<LaserFactory> m_laser_factory;
  std::unique_ptr<ProjectileFactory> m_bullet_factory;
  std::unique_ptr<WallFactory> m_wall_factory;
  std::unique_ptr<QuestFactory> m_quest_factory;

  TimedEventManager m_timers;

  ToolBoxUI m_ui;

  std::deque<GameLevel> m_levels;
};

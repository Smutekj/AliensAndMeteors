#ifndef BOIDS_GAME_H
#define BOIDS_GAME_H

#include <unordered_set>
#include <numeric>
#include <fstream>

#include "Utils/Grid.h"
#include "Utils/RandomTools.h"

#include "BoidSystem.h"
#include "BulletSystem.h"
#include "PolygonObstacleManager.h"
#include "ExplosionEffect.h"

#include "core.h"
#include "SoundModule.h"
#include "Selection.h"
#include "Particles.h"

#include "GameState.h"

class UI;

enum class Context
{
  MAINMENU,
  SETTINGS,
  HIGHSCORE,
  GAME
};


class Game
{

public:
  int score = 0;

  enum class GameState
  {
    RUNNING,
    WON,
    PLAYER_DIED
  };

  Game(sf::RenderWindow &window, KeyBindings &bindings);

  void update(const float dt, sf::RenderWindow &win);

  GameState getState() const
  {
    return state;
  }

  void handleEvent(const sf::Event &event);

  void parseInput(sf::RenderWindow &window);

  void draw(sf::RenderWindow &window);

  void onMeteorDestruction(int entity_ind);

private:

  void drawUI(sf::RenderWindow &window);
  void moveView(sf::RenderWindow &window);
  void parseEvents(sf::RenderWindow &window);

  GameState state = GameState::RUNNING;

  int meteor_spawner_time = 0;
  Bloom bloom;
  Ligthning lights;

  sf::RenderWindow &m_window;

  KeyBindings &key_binding;

  Player player;
  sf::RectangleShape player_shape;

  BoidSystem boid_world;
  BulletSystem bullet_world;
  PolygonObstacleManager poly_manager;

  EffectsManager effects;

  int boss_spawner_timer = 0;
  int normal_spawner_timer = 0;
  int group_spawner_timer = 0;
  int power_spawner_timer = 0;

  std::unique_ptr<Particles> player_particles_left;
  std::unique_ptr<Particles> player_particles_right;

  sf::Font font;
  sf::Text health_text;
  TextureHolder textures;

  sf::RenderTexture t;
  sf::RenderTexture light_texture;

  sf::View default_view;


  friend UI;
};

#endif // BOIDS_GAME_H
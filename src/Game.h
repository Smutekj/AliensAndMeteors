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

class ScoreBoard
{

  std::map<int, std::vector<std::string>, std::greater<int>> score2players;

  int n_shown_scores = 10;

  std::string score_file;

public:
  ScoreBoard(std::string score_file)
      : score_file(score_file)
  {
    readScoresFromFile(score_file);
  }

  ~ScoreBoard()
  {
    writeScoresToFile(score_file);
  }

  void addScore(std::string player_name, int score)
  {

    if (score2players.count(score) > 0)
    {
      score2players.at(score).push_back(player_name);
    }
    else
    {
      score2players[score] = {player_name};
    }
  }

private:
  void readScoresFromFile(std::string score_file)
  {
    std::ifstream file(score_file);

    std::string line;
    while (std::getline(file, line))
    {
      std::string player_name;
      std::stringstream ss(line);
      ss >> player_name;
      int score;
      ss >> score;
      score2players[score].push_back(player_name);
    }
    file.close();
  }

  void writeScoresToFile(std::string score_file)
  {
    std::ofstream file(score_file);

    for (auto &[score, players] : score2players)
    {
      for (auto &player : players)
      {
        file << player << " " << score << "\n";
      }
    }
    file.close();
  }
};

class Settings
{

  int volume;

public:
  Settings() = default;

  void setVolume(int new_volume)
  {
    volume = new_volume;
  }
  int getVolume() const
  {
    return volume;
  }
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

  std::unique_ptr<Particles> player_particles;

  sf::Font font;
  sf::Text health_text;
  TextureHolder textures;

  sf::RenderTexture t;
  sf::RenderTexture light_texture;

  sf::View default_view;


  friend UI;
};

#endif // BOIDS_GAME_H
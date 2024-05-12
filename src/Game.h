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

  Player player;
  sf::RectangleShape player_shape;

  bool selection_pending = false;
  std::vector<int> selection;
  sf::Vector2f start_position;
  sf::Vector2f end_position;
  std::vector<int> selected_meteors;

  BoidSystem boid_world;
  BulletSystem bullet_world;
  PolygonObstacleManager poly_manager;

  EffectsManager effects;

  int boss_spawner_timer = 0;
  int normal_spawner_timer = 0;
  int group_spawner_timer = 0;

  std::unique_ptr<Particles> player_particles;

  sf::Font font;
  sf::Text health_text;
  TextureHolder textures;

  sf::RenderTexture t;
  sf::RenderTexture light_texture;

  friend UI;

  enum class EndGameType
  {
    DIED,
    WON,
  };

  int meteor_spawner_time = 0;
  Bloom bloom;
  Ligthning lights;

  sf::RenderWindow& m_window;

public:
  std::vector<sf::Vector2f> inters_test;
  sf::Vector2f click_position = {0, 0};

  bool game_is_running = true;
  bool game_is_stopped_ = false;
  bool last_pressed_was_space = false;


  Game(sf::RenderWindow &window);

  void update(const float dt, sf::RenderWindow &win);

  void handleEvent(const sf::Event& event);

  void endGame(EndGameType end_type)
  {
    game_is_running = false;
  }

  void addExplosion(sf::Vector2f at, float radius, int type = 0)
  {
    effects.createExplosion(at, radius);
  }

  void run()
  {
  }

  void parseInput(sf::RenderWindow &window);

  void draw(sf::RenderWindow& window);

  void addEnemy(sf::Vector2f at);

  void onMeteorDestruction(int entity_ind)
  {
    auto &meteor = poly_manager.meteors.at(entity_ind);
    poly_manager.destroyMeteor(entity_ind);
    return;

    if (meteor.radius < 3.f)
    {
      return; //! no shattering of small meteors
    }

    auto old_center = meteor.getCenter();
    const auto &old_points = meteor.points;

    int n_points = old_points.size();
    std::vector<Polygon> new_meteors;
    int first_point = 0;
    int last_point = 1;
    while (last_point != 0)
    {

      first_point = last_point;
      int delta_max = n_points - first_point - 1;
      last_point = first_point + (rand() % 5 + 1);

      if (last_point >= n_points)
      {
        last_point = 0;
        new_meteors.push_back({n_points - first_point + 2});
        auto &new_meteor = new_meteors.back();
        new_meteor.points.at(0) = old_center;
        for (int k = first_point; k <= n_points - 1; ++k)
        {
          new_meteor.points.at(k - first_point + 1) = meteor.points.at(k);
        }
        new_meteor.points.at(n_points - first_point + 1) = meteor.points.at(0);

        new_meteor.vel = (new_meteor.getCenter() - old_center);
        poly_manager.addMeteor(new_meteor);

        break;
      }

      new_meteors.push_back({last_point - first_point + 2});
      auto &new_meteor = new_meteors.back();
      new_meteor.setPosition(meteor.getPosition());
      new_meteor.setRotation(meteor.getRotation());
      new_meteor.setScale(meteor.getScale());
      new_meteor.radius = meteor.getScale().x;
      new_meteor.points.at(0) = old_center;
      for (int k = first_point; k <= last_point; ++k)
      {
        new_meteor.points.at(k - first_point + 1) = meteor.points.at(k);
      }
      new_meteor.vel = (new_meteor.getCenter() - old_center);
      poly_manager.addMeteor(new_meteor);
    }
  }

  void spawnBullet(sf::Vector2f at, sf::Vector2f vel, Player *player = nullptr)
  {
    bullet_world.spawnBullet(-1, at, vel, player);
  }

  void addGroupOfEnemies(sf::Vector2f at, float radius, int n_in_group);

private:
  void drawUI(sf::RenderWindow &window);
  void moveView(sf::RenderWindow &window);
  void parseEvents(sf::RenderWindow &window);
};

#endif // BOIDS_GAME_H
#ifndef BOIDS_GAME_H
#define BOIDS_GAME_H

#include <unordered_set>
#include <numeric>

#include "Utils/Grid.h"
#include "Utils/RandomTools.h"

#include "BoidSystem.h"
#include "BulletSystem.h"
#include "PolygonObstacleManager.h"
#include "ExplosionEffect.h"

#include "core.h"
#include "SoundModule.h"
#include "Selection.h"

class UI;

class Game
{

  Player player;
  sf::RectangleShape player_shape;

  sf::VertexArray boid_vertices;
  sf::VertexArray bullet_vertices;

  bool selection_pending = false;
  std::vector<int> selection;
  sf::Vector2f start_position;
  sf::Vector2f end_position;
  std::vector<int> selected_meteors;

  BoidSystem boid_world;
  BulletSystem bullet_world;
  PolygonObstacleManager poly_manager;

  std::map<int, std::unique_ptr<ExplosionEffect>> effects;

  friend UI;

  enum class EndGameType{
    DIED,
    WON,
  };

  int meteor_spawner_time = 0;

public:


  sf::Vector2f click_position = {0, 0};

  bool game_is_running = true;
  bool game_is_stopped_ = false;
  bool last_pressed_was_space = false;

  Game(sf::Vector2i n_cells, sf::Vector2f box_size, sf::RenderWindow& window);

  void update(const float dt, sf::RenderWindow &win);

  void endGame(EndGameType end_type){
    game_is_running = false;

  }

  void addExplosion(sf::Vector2f at, int type)
  {
    int effect_ind = 0;
    if (!effects.empty())
    {
      effect_ind = effects.rbegin()->first + 1;
    }
    effects[effect_ind] = std::make_unique<ExplosionEffect>(at, 20.f);
  }


void
parseInput(sf::RenderWindow &window);

void draw(sf::RenderWindow &window);

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
void moveView(sf::RenderWindow &window);
void parseEvents(sf::RenderWindow &window);
}
;

#endif // BOIDS_GAME_H
#ifndef BOIDS_GAME_H
#define BOIDS_GAME_H

#include <SFML/Graphics/RectangleShape.hpp>

#include "Commands.h"
#include "GameWorld.h"
#include "UI.h"
#include "Bloom.h"
#include "ObjectiveSystem.h"

class GameWorld;

class Game
{

public:
  enum class GameState
  {
    RUNNING,
    WON,
    PLAYER_DIED
  };

  Game(sf::RenderWindow &window, KeyBindings &bindings);

  void update(const float dt, sf::RenderWindow &win);
  void handleEvent(const sf::Event &event);
  void parseInput(sf::RenderWindow &window);
  void draw(sf::RenderWindow &window);

  int getScore() const;
  GameState getState() const;

private:
  void drawUI(sf::RenderWindow &window);
  void moveView(sf::RenderWindow &window);
  void parseEvents(sf::RenderWindow &window);

  void spawnNextObjective();
  void spawnBossObjective();
  void addDestroyNObjective(ObjectType type, int count);

  ObjectiveSystem m_objective_system;

  int m_score = 0;

  GameState m_state = GameState::RUNNING;

  Bloom m_bloom;

  sf::RenderWindow &m_window;

  KeyBindings &m_key_binding;

  PlayerEntity *m_player;

  std::unique_ptr<GameWorld> m_world;

  sf::RenderTexture m_bloom_texture;

  sf::View m_default_view;

  sf::Font m_font;
  sf::Text m_health_text;

  TextureHolder m_textures;

  friend UI;
  UI m_ui;
};

#endif // BOIDS_GAME_H
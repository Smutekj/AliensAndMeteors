#ifndef BOIDS_GAME_H
#define BOIDS_GAME_H

#include <unordered_set>
#include <numeric>
#include <fstream>

#include "Commands.h"
#include "GameWorld.h"
#include "UI.h"
#include "Bloom.h"


class GameWorld;

class Game
{

public:
  int m_score = 0;

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

private:

  void drawUI(sf::RenderWindow &window);
  void moveView(sf::RenderWindow &window);
  void parseEvents(sf::RenderWindow &window);

  GameState state = GameState::RUNNING;

  Bloom bloom;

  sf::RenderWindow &m_window;

  KeyBindings &key_binding;

  PlayerEntity* m_player;

  std::unique_ptr<GameWorld> m_world;

  UI m_ui;

  sf::Font font;
  sf::Text health_text;

  sf::RenderTexture t;
  sf::RenderTexture light_texture;

  sf::View default_view;

  friend UI;
};

#endif // BOIDS_GAME_H
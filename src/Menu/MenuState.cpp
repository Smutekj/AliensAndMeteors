#include "MenuState.h"

#include <cmath>

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/RectangleShape.hpp>

#include "ScoreBoard.h"
#include "StateStack.h"

MenuState::MenuState(StateStack &stack, Context &context)
    : State(stack, context), m_menu(context.font)
{
  m_background_texture.loadFromFile("../Resources/Starbasesnow.png");
  m_background_texture.setRepeated(true);
  m_background_texture.setSmooth(true);

  sf::Vector2f window_size = {static_cast<float>(context.window->getSize().x),
                              static_cast<float>(context.window->getSize().y)};
  m_background_rect.setSize(window_size);
  m_background_rect.setTexture(&m_background_texture);
  m_background_rect.setTextureRect({0, 0, (int)m_background_texture.getSize().x / 2, (int)m_background_texture.getSize().y / 2});

  auto new_game_button = std::make_unique<ChangeStateItem>(context, m_stack, States::ID::Game, States::ID::None, "New Game");
  auto settings_button = std::make_unique<ChangeStateItem>(context, m_stack, States::ID::Settings, States::ID::Menu);
  auto exit_button = std::make_unique<ChangeStateItem>(context, m_stack, States::ID::Exit);
  auto score_button = std::make_unique<ChangeStateItem>(context, m_stack, States::ID::Score, States::ID::Menu, "High Scores");

  m_menu.addItem(std::move(new_game_button));
  m_menu.addItem(std::move(settings_button));
  m_menu.addItem(std::move(score_button));
  m_menu.addItem(std::move(exit_button));

  m_context.window->setView(m_context.window->getDefaultView());
}

MenuState::~MenuState() {}

void MenuState::update(float dt)
{
  //! animate background
  m_background_animation_time += 1;
  int texture_size_x = m_background_texture.getSize().x;
  auto texture_rect = m_background_rect.getTextureRect();

  //! make texture rect go from left to right and then back
  texture_rect.left = (-std::abs(m_background_animation_time - texture_size_x / 2) + texture_size_x / 2) % texture_size_x;
  texture_rect.top = 100 * std::sin(m_background_animation_time / 200.f);
  m_background_rect.setTextureRect(texture_rect);
}

void MenuState::handleEvent(const sf::Event &event)
{

  auto &window = *m_context.window;

  m_menu.handleEvent(event);
}

void MenuState::draw()
{

  auto &window = *m_context.window;

  window.draw(m_background_rect);
  m_menu.draw(window);
}

EndScreenState::EndScreenState(StateStack &stack, Context &context)
    : State(stack, context)
{
  m_goodbye_text.setFont(*context.font);
}

EndScreenState::~EndScreenState() {}

void EndScreenState::update(float dt)
{
  m_timer--;
  if (m_timer < 0)
  {
    m_context.window->close();
  }
}

void EndScreenState::handleEvent(const sf::Event &event)
{
  if (event.type == sf::Event::KeyReleased)
  {
    m_context.window->close();
  }
}

void EndScreenState::draw()
{

  auto &window = *m_context.window;
  window.setView(window.getDefaultView());

  sf::Vector2f window_size = {static_cast<float>(window.getSize().x), static_cast<float>(window.getSize().y)};

  m_goodbye_text.setString("Good Bye!");
  m_goodbye_text.setFillColor(sf::Color::Blue);
  m_goodbye_text.setScale({2.f, 2.f});
  Menu::centerTextInWindow(window, m_goodbye_text, window.getSize().y / 2.f);
  window.draw(m_goodbye_text);
}

PlayerDiedState::PlayerDiedState(StateStack &stack, Context &context)
    : State(stack, context), m_menu(context.font)
{
  m_text.setFont(*context.font);

  auto enter_name = std::make_unique<EnterTextItem>(context, m_entered_name, "Enter Your Name:");
  m_menu.addItem(std::move(enter_name));

  sf::Vector2f window_size = {static_cast<float>(context.window->getSize().x),
                              static_cast<float>(context.window->getSize().y)};
  m_menu.setYPos(window_size.y * 7. / 10.);
}

PlayerDiedState::~PlayerDiedState() {}

void PlayerDiedState::update(float dt)
{
}

void PlayerDiedState::handleEvent(const sf::Event &event)
{
  if (event.type == sf::Event::KeyReleased && event.key.code == sf::Keyboard::Enter)
  {
    if (m_entered_name.length() != 0)
    {
      m_context.score->setScore(m_entered_name, m_context.score->getCurrentScore());
      m_stack->popState();
      m_stack->pushState(States::ID::Menu);
    }
  }

  m_menu.handleEvent(event);
}

void PlayerDiedState::draw()
{
  auto &window = *m_context.window;
  window.setView(window.getDefaultView());

  m_menu.draw(window);

  sf::Vector2f window_size = {static_cast<float>(window.getSize().x), static_cast<float>(window.getSize().y)};

  m_text.setString("You Died!");
  m_text.setFillColor(sf::Color::Blue);
  m_text.setScale({2.f, 2.f});
  m_text.setPosition({window_size.x / 2.f - m_text.getGlobalBounds().width / 2.f,
                      window_size.y / 4.f});
  window.draw(m_text);

  m_text.setString("Your score was: " + std::to_string(m_context.score->getCurrentScore()));
  m_text.setFillColor(sf::Color::Blue);
  m_text.setScale({2.f, 2.f});
  m_text.setPosition({window_size.x / 2.f - m_text.getGlobalBounds().width / 2.f,
                      window_size.y / 2.f + m_text.getGlobalBounds().height});
  window.draw(m_text);
}

ScoreBoardState::ScoreBoardState(StateStack &stack, Context &context)
    : State(stack, context)
{
  m_left_text.setFont(*context.font);
  m_right_text.setFont(*context.font);
  m_left_text.setFillColor(sf::Color::Green);
  m_right_text.setFillColor(sf::Color::Red);
}

ScoreBoardState::~ScoreBoardState() {}

void ScoreBoardState::update(float dt)
{
}

void ScoreBoardState::handleEvent(const sf::Event &event)
{
  if (event.type == sf::Event::KeyReleased && event.key.code == sf::Keyboard::Escape)
  {
    m_stack->popState();
  }
}

void ScoreBoardState::draw()
{

  auto &window = *m_context.window;
  auto old_view = window.getView();
  window.setView(window.getDefaultView());

  const auto &scores = *m_context.score;

  sf::Vector2f window_size = {static_cast<float>(window.getSize().x), static_cast<float>(window.getSize().y)};

  float item_width = window_size.x / 5.f;
  float y_pos = 100.f;

  m_left_text.setScale({1.1f, 1.1f});
  m_right_text.setScale({1.1f, 1.1f});
  drawScoreLine(window, "Player", "Score", item_width, y_pos);
  y_pos += m_left_text.getGlobalBounds().height * 1.55f;
  m_left_text.setScale({1, 1});
  m_right_text.setScale({1, 1});

  float line_height = m_context.font->getLineSpacing(m_left_text.getCharacterSize()) * 1.05f;

  for (auto &[score, player_names] : scores.getAllScores())
  {
    for (auto &player_name : player_names)
    {
      drawScoreLine(window, player_name, std::to_string(score), item_width, y_pos);
      y_pos += line_height;
    }
  }

  window.setView(old_view);
}

void ScoreBoardState::drawScoreLine(sf::RenderWindow &window,
                                    std::string text1, std::string text2, float width, float y_position)
{
  m_left_text.setString(text1);
  m_left_text.setPosition({window.getSize().x / 2.f - width, y_position});

  m_right_text.setString(text2);
  m_right_text.setPosition({window.getSize().x / 2.f + width - m_right_text.getGlobalBounds().width, y_position});
  window.draw(m_left_text);
  window.draw(m_right_text);
}
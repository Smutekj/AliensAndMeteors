#include "MenuState.h"

#include <cmath>

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/RectangleShape.hpp>

#include "ScoreBoard.h"
#include "StateStack.h"


MenuState::MenuState(StateStack &stack, Context &context)
    : State(stack, context), m_menu(context.font)
{
  m_is_final_state = true;

  background_texture.loadFromFile("../Resources/Starbasesnow.png");
  background_texture.setRepeated(true);
  background_texture.setSmooth(true);

  sf::Vector2f window_size = {static_cast<float>(context.window->getSize().x),
                              static_cast<float>(context.window->getSize().y)};
  background_rect.setSize(window_size);
  background_rect.setTexture(&background_texture);
  background_rect.setTextureRect({0, 0, (int)background_texture.getSize().x / 2, (int)background_texture.getSize().y / 2});

  auto new_game_button = std::make_unique<ChangeStateItem>(context, m_stack, States::ID::Game);
  new_game_button->m_text = "New Game";
  auto settings_button = std::make_unique<ChangeStateItem>(context, m_stack, States::ID::Settings, States::ID::Menu);
  auto exit_button = std::make_unique<ChangeStateItem>(context, m_stack, States::ID::Exit);
  auto score_button = std::make_unique<ChangeStateItem>(context, m_stack, States::ID::Score, States::ID::Menu);

  exit_button->m_text = "Exit";

  m_menu.addItem(std::move(new_game_button));
  m_menu.addItem(std::move(settings_button));
  m_menu.addItem(std::move(score_button));
  m_menu.addItem(std::move(exit_button));
}

MenuState::~MenuState() {}

void MenuState::update(float dt)
{
  //! animate background
  background_animation_time += 1;
  int texture_size_x = background_texture.getSize().x;
  auto texture_rect = background_rect.getTextureRect();

  //! make texture rect go from left to right and then back
  texture_rect.left = (-std::abs(background_animation_time - texture_size_x / 2) + texture_size_x / 2) % texture_size_x;
  texture_rect.top = 100 * std::sin(background_animation_time / 200.f);
  background_rect.setTextureRect(texture_rect);
}

void MenuState::handleEvent(const sf::Event &event)
{

  auto &window = *m_context.window;

  m_menu.handleEvent(event);
}

void MenuState::draw()
{

  auto &window = *m_context.window;

  window.draw(background_rect);
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

  auto &window = *getContext().window;

  sf::Vector2f window_size = {static_cast<float>(window.getSize().x), static_cast<float>(window.getSize().y)};

  m_goodbye_text.setString("Good Bye!");
  m_goodbye_text.setFillColor(sf::Color::Blue);
  m_goodbye_text.setScale({2.f, 2.f});
  m_goodbye_text.setPosition({window_size.x / 2.f - m_goodbye_text.getGlobalBounds().width / 2.f, window_size.y / 2.f});

  window.draw(m_goodbye_text);
}

PlayerDiedState::PlayerDiedState(StateStack &stack, Context &context)
    : State(stack, context)
{
  m_text.setFont(*context.font);
}

PlayerDiedState::~PlayerDiedState() {}

void PlayerDiedState::update(float dt)
{
  if (m_dash_visibility_time-- <= 0)
  {
    m_dash_visibility_time = m_dash_visibility_cooldown;
    m_dash_is_visible = !m_dash_is_visible;
  }
}

void PlayerDiedState::handleEvent(const sf::Event &event)
{
  if (event.type == sf::Event::KeyReleased && event.key.code == sf::Keyboard::Escape && !m_is_entering_text)
  {
    m_stack->popState();
    m_stack->pushState(States::ID::Menu);
  }
  if (m_is_entering_text)
  {
    if (event.type == sf::Event::KeyReleased)
    {
      m_dash_visibility_time = m_dash_visibility_cooldown;
      m_dash_is_visible = true;
      if (event.key.code >= sf::Keyboard::A && event.key.code <= sf::Keyboard::Z)
      {
        char character = event.key.shift ? 'A' : 'a';
        character += static_cast<int>(event.key.code);
        m_entered_name.push_back(character);
      }
      if (event.key.code == sf::Keyboard::BackSpace && m_entered_name.size() > 0)
      {
        m_entered_name.pop_back();
      }
      if (event.key.code == sf::Keyboard::Enter)
      {
        m_context.score->setScore(m_entered_name, m_context.score->getCurrentScore());
        m_is_entering_text = false;
      }
    }
  }
}

void PlayerDiedState::draw()
{
  auto &window = *getContext().window;
  window.setView(window.getDefaultView());

  sf::Vector2f window_size = {static_cast<float>(window.getSize().x), static_cast<float>(window.getSize().y)};

  m_text.setString("You Died!");
  m_text.setFillColor(sf::Color::Blue);
  m_text.setScale({2.f, 2.f});
  m_text.setPosition({window_size.x / 2.f - m_text.getGlobalBounds().width / 2.f,
                      window_size.y / 4.f});
  window.draw(m_text);

  if (m_is_entering_text)
  {
    m_text.setString("Enter your name!");
    m_text.setFillColor(sf::Color::Blue);
    m_text.setScale({2.f, 2.f});
    m_text.setPosition({window_size.x / 2.f - m_text.getGlobalBounds().width / 2.f,
                        window_size.y / 2.f - m_text.getGlobalBounds().height});
    window.draw(m_text);

    m_text.setFillColor(sf::Color::Red);
    m_text.setScale({2.f, 2.f});
    m_text.setPosition({window_size.x / 2.f - m_text.getGlobalBounds().width / 2.f,
                        window_size.y / 2.f + m_text.getGlobalBounds().height});

    m_dash_is_visible ? m_text.setString(m_entered_name + "_") : m_text.setString(m_entered_name);
    window.draw(m_text);

    return;
  }

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
  m_left_text.setScale({1,1});
  m_right_text.setScale({1, 1});


  for (auto &[score, player_names] : scores.getAllScores())
  {
    for (auto &player_name : player_names)
    {
      drawScoreLine(window, player_name, std::to_string(score), item_width, y_pos);
      y_pos += m_left_text.getGlobalBounds().height * 1.05f;
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
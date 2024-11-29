#include "MenuState.h"

#include <cmath>

#include <Window.h>

#include "ScoreBoard.h"
#include "StateStack.h"

MenuState::MenuState(StateStack &stack, Context &context)
    : State(stack, context), m_menu(context.font)
{
  m_background_texture.loadFromFile("../Resources/Textures/background.png");
  // m_background_texture.setRepeated(true);
  // m_background_texture.setSmooth(true);

  utils::Vector2f window_size = {static_cast<float>(context.window->getTargetSize().x),
                                 static_cast<float>(context.window->getTargetSize().y)};
  m_background_rect.setScale(window_size * 2.);
  m_background_rect.setTexture(m_background_texture);

  auto new_game_button = std::make_unique<ChangeStateItem>(context, m_stack, States::ID::Game, States::ID::None, "New Game");
  auto settings_button = std::make_unique<ChangeStateItem>(context, m_stack, States::ID::Settings, States::ID::Menu);
  auto exit_button = std::make_unique<ChangeStateItem>(context, m_stack, States::ID::Exit);
  auto score_button = std::make_unique<ChangeStateItem>(context, m_stack, States::ID::Score, States::ID::Menu, "High Scores");

  m_menu.addItem(std::move(new_game_button));
  m_menu.addItem(std::move(settings_button));
  m_menu.addItem(std::move(score_button));
  m_menu.addItem(std::move(exit_button));

  m_context.window->m_view = m_context.window->getDefaultView();
}

MenuState::~MenuState() {}

void MenuState::update(float dt)
{
  //! animate background
  m_background_animation_time += 1;
  int texture_size_x = m_background_rect.m_tex_rect.width;
  auto texture_rect = m_background_rect.m_tex_rect;

  //! make texture rect go from left to right and then back
  texture_rect.pos_x = (-std::abs(m_background_animation_time - texture_size_x / 2) + texture_size_x / 2) % texture_size_x;
  texture_rect.pos_y = 100 * std::sin(m_background_animation_time / 200.f);
  m_background_rect.m_tex_rect = texture_rect;
}

void MenuState::handleEvent(const SDL_Event &event)
{

  auto &window = *m_context.window;

  m_menu.handleEvent(event);
}

void MenuState::draw()
{

  auto &window = *m_context.window;
  window.drawSprite(m_background_rect, "Instanced");
  window.drawAll();
  m_menu.draw(window);
}

EndScreenState::EndScreenState(StateStack &stack, Context &context)
    : State(stack, context)
{
  m_goodbye_text.setFont(context.font);
}

EndScreenState::~EndScreenState() {}

void EndScreenState::update(float dt)
{
  m_timer--;
  if (m_timer < 0)
  {
    m_context.window_handle->close();
  }
}

void EndScreenState::handleEvent(const SDL_Event &event)
{
  if (event.type == SDL_KEYUP)
  {
    m_context.window_handle->close();
  }
}

void EndScreenState::draw()
{

  auto &window = *m_context.window;
  window.m_view = window.getDefaultView();

  utils::Vector2f window_size = {static_cast<float>(window.getTargetSize().x), static_cast<float>(window.getTargetSize().y)};

  m_goodbye_text.setText("Good Bye!");
  m_goodbye_text.setColor({0, 0, 255, 255});
  m_goodbye_text.setScale(2.f, 2.f);
  Menu::centerTextInWindow(window, m_goodbye_text, window.getTargetSize().y / 2.f);
  window.drawText(m_goodbye_text, "Text");
}

PlayerDiedState::PlayerDiedState(StateStack &stack, Context &context)
    : State(stack, context), m_menu(context.font)
{
  m_text.setFont(context.font);

  auto enter_name = std::make_unique<EnterTextItem>(context, m_entered_name, "Enter Your Name:");
  m_menu.addItem(std::move(enter_name));

  utils::Vector2f window_size = {static_cast<float>(context.window->getTargetSize().x),
                                 static_cast<float>(context.window->getTargetSize().y)};
  m_menu.setYPos(window_size.y * 8. / 10.);
}

PlayerDiedState::~PlayerDiedState() {}

void PlayerDiedState::update(float dt)
{
}

void PlayerDiedState::handleEvent(const SDL_Event &event)
{
  if (event.type == SDL_KEYUP)
  {
    bool key_is_enter = event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_KP_ENTER;
    if (key_is_enter)
    {
      if (m_entered_name.length() != 0)
      {
        m_context.score->setScore(m_entered_name, m_context.score->getCurrentScore());
      }
      m_stack->popState();
      m_stack->pushState(States::ID::Menu);
    }
    else if (event.key.keysym.sym == SDLK_BACKSPACE && m_entered_name.size() > 0)
    {
      m_entered_name.pop_back();
    }
  }
  else if (event.type == SDL_TEXTINPUT)
  {
    m_entered_name = m_entered_name + event.text.text[0];
  }

  m_menu.handleEvent(event);
}

void PlayerDiedState::draw()
{
  auto &window = *m_context.window;

  window.m_view = window.getDefaultView();

  m_menu.draw(window);

  utils::Vector2f window_size = {static_cast<float>(window.getTargetSize().x), static_cast<float>(window.getTargetSize().y)};

  m_text.setText("You Died!");
  m_text.setColor({0, 0, 255, 255});
  m_text.setScale(2.f, 2.f);
  m_text.centerAround({window_size.x / 2.f,
                       window_size.y * 6.f / 8.f});
  window.drawText(m_text, "Text");

  m_text.setText("Your score was: " + std::to_string(m_context.score->getCurrentScore()));
  m_text.setColor({0, 0, 255, 255});
  m_text.setScale(2.f, 2.f);
  m_text.centerAround({window_size.x / 2.f,
                       window_size.y * 4.f / 8.f});
  window.drawText(m_text, "Text");

  window.drawAll();
}

ScoreBoardState::ScoreBoardState(StateStack &stack, Context &context)
    : State(stack, context)
{
  m_left_text.setFont(context.font);
  m_right_text.setFont(context.font);
  m_left_text.setColor({0, 255, 0, 255});
  m_right_text.setColor({255, 0, 0, 255});
}

ScoreBoardState::~ScoreBoardState() {}

void ScoreBoardState::update(float dt)
{
}

void ScoreBoardState::handleEvent(const SDL_Event &event)
{
  if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_ESCAPE)
  {
    m_stack->popState();
  }
}

void ScoreBoardState::draw()
{

  auto &window = *m_context.window;
  auto old_view = window.m_view;
  window.m_view = window.getDefaultView();

  const auto &scores = *m_context.score;

  utils::Vector2f window_size = {static_cast<float>(window.getTargetSize().x), static_cast<float>(window.getTargetSize().y)};

  float item_width = window_size.x / 5.f;
  float y_pos = window_size.y / 20.f + 20;

  m_left_text.setScale(1.1f, 1.1f);
  m_right_text.setScale(1.1f, 1.1f);
  drawScoreLine(window, "Player", "Score", item_width, y_pos);
  y_pos += window_size.y / 10.f; // m_left_text.getGlobalBounds().height * 1.55f;
  m_left_text.setScale(1, 1);
  m_right_text.setScale(1, 1);

  float line_height = 50.f; // m_context.font->getLineSpacing(m_left_text.getCharacterSize()) * 1.05f;

  for (auto &[score, player_names] : scores.getAllScores())
  {
    for (auto &player_name : player_names)
    {
      drawScoreLine(window, player_name, std::to_string(score), item_width, y_pos);
      y_pos += line_height;
    }
  }

  window.m_view = old_view;
}

void ScoreBoardState::drawScoreLine(Renderer &window,
                                    std::string text1, std::string text2,
                                    float width, float y_position)
{
  auto window_size = window.getTargetSize();
  m_left_text.setText(text1);
  m_left_text.centerAround({window_size.x / 2.f - width, y_position});

  m_right_text.setText(text2);
  m_right_text.centerAround({window_size.x / 2.f + width, y_position});
  window.drawText(m_left_text, "Text");
  window.drawText(m_right_text, "Text");
}
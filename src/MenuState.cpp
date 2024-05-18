#include "MenuState.h"
#include "State.h"

#include "SFML/Graphics/RenderWindow.hpp"
#include "SFML/Graphics/RectangleShape.hpp"

MenuState::MenuState(StateStack &stack, Context &context)
    : State(stack, context), m_menu(context.font)
{
  m_is_final_state = true;

  menu_font.loadFromFile("../Resources/DigiGraphics.ttf");
  field_text.setFont(menu_font);

  fields.push_back({MenuField::NEW_GAME, "NEW GAME", States::ID::Game, true});
  fields.push_back({MenuField::SETTINGS, "SETTINGS", States::ID::Settings, false});
  fields.push_back({MenuField::EXIT, "EXIT", States::ID::None, true});

  background_texture.loadFromFile("../Resources/Starbasesnow.png");
  background_texture.setRepeated(true);
  background_texture.setSmooth(true);

  auto new_game_button = std::make_unique<ChangeStateItem>(m_stack, States::ID::Game);
  new_game_button->m_text = "New Game";
  auto settings_button = std::make_unique<ChangeStateItem>(m_stack, States::ID::Settings);
  auto exit_button = std::make_unique<ChangeStateItem>(m_stack, States::ID::Exit);
  exit_button->m_text = "Exit";

  m_menu.addItem(std::move(new_game_button));
  m_menu.addItem(std::move(settings_button));
  m_menu.addItem(std::move(exit_button));
}

MenuState::~MenuState() {}

void MenuState::update(float dt)
{
}

void MenuState::handleEvent(const sf::Event &event)
{

  auto &window = *getContext().window;

  m_menu.handleEvent(event);
}

void MenuState::draw()
{


  auto &window = *getContext().window;
  m_menu.draw(window);

}

EndScreenState::EndScreenState(StateStack &stack, Context &context)
:
State(stack, context)
{
  m_goodbye_text.setFont(*context.font);
}

EndScreenState::~EndScreenState() {}

void EndScreenState::update(float dt)
{
  m_timer--;
  if(m_timer < 0)
  {
    m_context.window->close();
  }
}

void EndScreenState::handleEvent(const sf::Event &event)
{
  if(event.type == sf::Event::Closed)
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
  m_goodbye_text.setScale({2.f,2.f});
  m_goodbye_text.setPosition({window_size.x - m_goodbye_text.getGlobalBounds().width/2.f, window_size.y/2.f});

  window.draw(m_goodbye_text);
}
#include "MenuState.h"
#include "State.h"

#include "SFML/Graphics/RenderWindow.hpp"
#include "SFML/Graphics/RectangleShape.hpp"

MenuState::MenuState(StateStack &stack, Context &context) : State(stack, context)
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



}

MenuState::~MenuState() {}

void MenuState::update(float dt)
{
}

void MenuState::handleEvent(const sf::Event &event)
{

  auto &window = *getContext().window;

  if (event.type == sf::Event::KeyReleased)
  {
    if (event.key.code == sf::Keyboard::Up)
    {
      moveSelectionUp();
    }
    else if (event.key.code == sf::Keyboard::Down)
    {
      moveSelectionDown();
    }
    else if (event.key.code == sf::Keyboard::Enter)
    {
      if (selected_field == MenuField::EXIT)
      {
        window.close();
      }
      else
      {
        if(fields.at(selected_field).pop_current_state){
          requestStackPop();
        }
        requestStackPush(fields.at(selected_field).destination);
      }
    }
  }
}

void MenuState::draw()
{

  auto &window = *getContext().window;
  window.setView(window.getDefaultView());

  sf::RectangleShape background;
  sf::Vector2f size = {static_cast<float>(window.getSize().x), static_cast<float>(window.getSize().y)};
  background.setSize(size);
	background.setTexture(&background_texture);
  sf::Vector2i rect_size = {static_cast<int>(2*window.getSize().x), static_cast<int>(2*window.getSize().y)};
  background.setTextureRect({0,0,rect_size.x , rect_size.y});  
  window.draw(background);


  auto window_size = window.getSize();
  float field_y_pos = 100;

  for (const auto &field : fields)
  {
    if (field.field == selected_field)
    {
      field_text.setScale({1.1f, 1.1f});
      field_text.setFillColor(sf::Color::Red);
    }
    auto &text = field.text;
    field_text.setString(text);
    auto text_bound = field_text.getGlobalBounds();
    field_text.setPosition({window_size.x / 2.f - text_bound.width / 2.f, field_y_pos});
    field_y_pos += text_bound.height * 1.05f;
    window.draw(field_text);

    field_text.setScale({1.f, 1.f});
    field_text.setFillColor(sf::Color::White);
  }

}

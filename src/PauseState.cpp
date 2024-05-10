#include "PauseState.h"
#include "State.h"

#include "SFML/Graphics/RenderWindow.hpp"

PauseState::PauseState(StateStack &stack, Context &context) : State(stack, context)
{
    m_is_final_state = true;

    menu_font.loadFromFile("../Resources/DigiGraphics.ttf");
    field_text.setFont(menu_font);

    fields.push_back({MenuField::RESUME, "RESUME", States::ID::None, true});
    fields.push_back({MenuField::SETTINGS, "SETTINGS", States::ID::Settings, false});
    fields.push_back({MenuField::EXIT, "EXIT TO MENU", States::ID::Menu, true});
}

void PauseState::update(float dt)
{
}

void PauseState::handleEvent(const sf::Event &event)
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

            auto &selected_field_data = fields.at(selected_field);

            if (selected_field == MenuField::RESUME)
            {
                window.setView(old_view);
            }
            if (selected_field_data.pop_current_state)
            {
                requestStackPop();
            }
            if (selected_field == MenuField::EXIT)
            {
                requestStateClear();
            }

            requestStackPush(selected_field_data.destination);
        }
    }
}

void PauseState::draw()
{

    auto &window = *getContext().window;
    old_view = window.getView();

    window.setView(window.getDefaultView());

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

    window.setView(old_view);
}

#include "Menu.h"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/RectangleShape.hpp>

#include "Utils/magic_enum.hpp"
#include "Utils/magic_enum_utility.hpp"

#include <iostream>

void Menu::draw(sf::RenderWindow &window)
{
    auto old_view = window.getView();
    window.setView(window.getDefaultView());

    sf::Vector2f size = {static_cast<float>(window.getSize().x), static_cast<float>(window.getSize().y)};

    auto window_size = window.getSize();
    float field_y_pos = 100;
    m_text.setFillColor(sf::Color::Red);

    for (int item_ind = 0; item_ind < m_items.size(); item_ind++)
    {
        auto &field = m_items.at(item_ind);
        if (item_ind == selected_item_ind)
        {
            m_text.setScale({1.1f, 1.1f});
            m_text.setFillColor(sf::Color::Red);
        }
        else
        {
            m_text.setFillColor(sf::Color::White);
            m_text.setScale({1.f, 1.f});
        }

        m_text.setString(field->m_text);
        auto text_bound = m_text.getLocalBounds();
        m_text.setPosition({window_size.x / 2.f - text_bound.width / 2.f, field_y_pos});
        field_y_pos += text_bound.height * 1.05f;
        window.draw(m_text);
    }

    window.setView(old_view);
}

void Menu::addItem(std::unique_ptr<MenuItem> item)
{
    m_items.push_back(std::move(item));
}

void Menu::handleEvent(sf::Event event)
{
    if (event.type == sf::Event::KeyReleased)
    {
        if (event.key.code == sf::Keyboard::Up)
        {
            selected_item_ind = (selected_item_ind - 1 + m_items.size()) % m_items.size();
        }
        else if (event.key.code == sf::Keyboard::Down)
        {
            selected_item_ind = (selected_item_ind + 1) % m_items.size();
        }

         m_items.at(selected_item_ind)->handleEvent(event);
    }
}

ChangeStateItem::ChangeStateItem(StateStack *stack, States::ID destination, States::ID source)
 : m_source(source), m_destination(destination), p_stack(stack)
{
    std::string destination_name = static_cast<std::string>(magic_enum::enum_name(m_destination));
    m_text = (destination_name);
}

void ChangeStateItem::handleEvent(sf::Event event)
{
    if (event.key.code == sf::Keyboard::Enter)
    {
        if(m_source == States::ID::None) //! if there is no source we don't want to return
        {
            p_stack->popState();
        }
        p_stack->pushState(m_destination);
    }
}

ChangeKeyItem::ChangeKeyItem(std::string command_name, PlayerControl command, KeyBindings *bindings)
    : m_command(command), m_command_name(command_name), p_bindings(bindings)
{
    std::string key_name = static_cast<std::string>(magic_enum::enum_name((*p_bindings)[m_command]));
    m_text = (command_name + " " + key_name);
}

void ChangeKeyItem::handleEvent(sf::Event event)
{
    if (event.key.code == sf::Keyboard::Enter)
    {
        if (!m_is_changing_key)
        {
            m_is_changing_key = true;
            m_text = (m_command_name + " ...");
        }
    }
    else if(m_is_changing_key)
    {
        p_bindings->setBinding(m_command, event.key.code);
        std::string key_name = static_cast<std::string>(magic_enum::enum_name(event.key.code));
        m_text = (m_command_name + " " + key_name);
        m_is_changing_key = false;
    }
}
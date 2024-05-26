#include "Menu.h"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/RectangleShape.hpp>

#include "../Utils/magic_enum.hpp"
#include "../Utils/magic_enum_utility.hpp"

#include "StateStack.h"

MenuItem::MenuItem(sf::Font *font) : p_font(font)
{
}

void MenuItem::setFillColor(sf::Color color)
{
    m_text_color = color;
}

sf::Vector2f MenuItem::getSize() const
{
    return m_item_size;
}

Menu::Menu(sf::Font *font) : p_font(font)
{
    m_text.setFont(*p_font);
}

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
        if (item_ind == m_selected_item_ind)
        {
            field->setScale({1.1f, 1.1f});
            field->setFillColor(sf::Color::Red);
        }
        else
        {
            field->setFillColor(sf::Color::White);
            field->setScale({1.f, 1.f});
        }

        m_text.setString(field->m_text);
        field->setPosition({window_size.x / 2.f, field_y_pos});
        field->draw(window);

        field_y_pos += field->getSize().y;
        // window.draw(m_text);
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
        if (event.key.code == sf::Keyboard::Up) //! select item above
        {
            m_items.at(m_selected_item_ind)->m_is_selected = false;
            m_selected_item_ind = (m_selected_item_ind - 1 + m_items.size()) % m_items.size();
            m_items.at(m_selected_item_ind)->m_is_selected = true;
        }
        else if (event.key.code == sf::Keyboard::Down) //! select item below
        {
            m_items.at(m_selected_item_ind)->m_is_selected = false;
            m_selected_item_ind = (m_selected_item_ind + 1) % m_items.size();
            m_items.at(m_selected_item_ind)->m_is_selected = true;
        }

        m_items.at(m_selected_item_ind)->handleEvent(event); 
    }
}

ChangeStateItem::ChangeStateItem(State::Context &context, StateStack *stack, States::ID destination, States::ID source)
    : m_source(source), m_destination(destination), p_stack(stack), MenuItem(context.font)
{
    std::string destination_name = static_cast<std::string>(magic_enum::enum_name(m_destination));
    m_text = (destination_name);
}

void ChangeStateItem::handleEvent(sf::Event event)
{
    if (event.key.code == sf::Keyboard::Enter)
    {
        if (m_source == States::ID::None) //! if there is no source we don't want to return
        {
            p_stack->popState();
        }
        p_stack->pushState(m_destination);
    }
}

ChangeKeyItem::ChangeKeyItem(std::string command_name, PlayerControl command, State::Context &context)
    : m_command(command), m_command_name(command_name), p_bindings(context.bindings), MenuItem(context.font)
{
    key_name = static_cast<std::string>(magic_enum::enum_name((*p_bindings)[m_command]));
    m_item_size.x = 200.f;
    sf::Text text;
    text.setFont(*context.font);
    text.setString("KEK");
    m_item_size.y = text.getGlobalBounds().width * 1.1f;
}

void ChangeKeyItem::handleEvent(sf::Event event)
{

    if (event.key.code == sf::Keyboard::Enter)
    {
        if (!m_is_changing_key)
        {
            m_is_changing_key = true;
            // p_bindings->unsetCommand(m_command);
        }
    }
    else if (m_is_changing_key)
    {
        if (p_bindings->setBinding(m_command, event.key.code)) //! if we succesfully changed key
        {
            key_name = static_cast<std::string>(magic_enum::enum_name(event.key.code));
            m_is_changing_key = false;
        }
    }
}

void ChangeStateItem::draw(sf::RenderWindow &window)
{
    sf::Text text;
    text.setFont(*p_font);
    text.setString(m_text);
    text.setPosition({getPosition().x - text.getLocalBounds().width / 2.f, getPosition().y});
    text.setFillColor(m_text_color);

    window.draw(text);
}

void ChangeKeyItem::draw(sf::RenderWindow &window)
{
    sf::Text left_text;
    left_text.setFont(*p_font);
    sf::Vector2f left_text_pos = {
        getPosition().x - m_item_size.x,
        getPosition().y};
    left_text.setString(m_command_name);
    left_text.setPosition(left_text_pos);
    left_text.setFillColor(m_text_color);
    sf::Text right_text;
    right_text.setFont(*p_font);

    // if(p_bindings->commandNotSet(m_command))
    // {
    //     m_is_changing_key = true;
    // }

    if (m_is_changing_key)
    {
        right_text.setString("...");
    }
    else
    {
        key_name = static_cast<std::string>(magic_enum::enum_name((*p_bindings)[m_command]));
        right_text.setString(key_name);
    }

    right_text.setFillColor(m_text_color);

    sf::Vector2f right_text_pos = {
        getPosition().x + m_item_size.x - right_text.getGlobalBounds().width,
        getPosition().y};
    right_text.setPosition(right_text_pos);

    window.draw(left_text);
    window.draw(right_text);
}
#pragma once

#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/Text.hpp>

#include "State.h"
#include "Commands.h"


class MenuItem : public sf::Transformable
{

protected:
    sf::Vector2f m_item_size = {300, 50};
    sf::Font *p_font;
    sf::Color text_color;
public:
    std::string m_text;
    bool m_is_selected = false;

    MenuItem(sf::Font *font) : p_font(font)
    {}

    void setFillColor(sf::Color color)
    {
        text_color = color;
    }

    sf::Vector2f getSize()const
    {
        return m_item_size;
    }

    virtual void handleEvent(sf::Event event) = 0;
    virtual void draw(sf::RenderWindow &window) = 0;
};

class ChangeStateItem : public MenuItem
{

    States::ID m_source;
    States::ID m_destination;
    StateStack *p_stack;

public:
    ChangeStateItem( State::Context &context, StateStack *stack, States::ID destination, States::ID source = States::ID::None);
    virtual void handleEvent(sf::Event event) override;
    virtual void draw(sf::RenderWindow &window) override;
};

class ChangeKeyItem : public MenuItem
{
    bool m_is_changing_key = false;
    PlayerControl m_command;
    std::string m_command_name;
    std::string key_name;

    KeyBindings *p_bindings;

public:

    ChangeKeyItem(std::string command_name, PlayerControl command, State::Context &context);
    virtual void handleEvent(sf::Event event) override;
    virtual void draw(sf::RenderWindow &window)override;
};

class Menu
{
    sf::Font *p_font;

    int selected_item_ind = 0;

    sf::Text m_text;

public:
    Menu(sf::Font *font) : p_font(font)
    {
        m_text.setFont(*p_font);
    }

    void draw(sf::RenderWindow &window);
    void addItem(std::unique_ptr<MenuItem> item);
    void handleEvent(sf::Event event);

private:
    std::vector<std::unique_ptr<MenuItem>> m_items;
};
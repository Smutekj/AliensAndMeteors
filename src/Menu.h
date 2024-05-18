#pragma once

#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/Text.hpp>

#include "StateStack.h"
#include "Commands.h"

class MenuItem
{

public:
    std::string m_text;
    bool m_is_selected = false;

    virtual void handleEvent(sf::Event event) = 0;
};

class ChangeStateItem : public MenuItem
{

    States::ID m_source;
    States::ID m_destination;
    StateStack *p_stack;

public:
    ChangeStateItem(StateStack *stack, States::ID destination, States::ID source = States::ID::None);
    virtual void handleEvent(sf::Event event) override;
};

class ChangeKeyItem : public MenuItem
{
    bool m_is_changing_key = false;
    PlayerControl m_command;
    std::string m_command_name;

    KeyBindings *p_bindings;

public:
    ChangeKeyItem(std::string command_name, PlayerControl command, KeyBindings *bindings);
    virtual void handleEvent(sf::Event event) override;
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
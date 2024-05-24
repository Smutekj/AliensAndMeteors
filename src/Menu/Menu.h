#pragma once

#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/Text.hpp>

#include "State.h"
#include "../Commands.h"


class MenuItem : public sf::Transformable
{

protected:
    sf::Vector2f m_item_size = {300, 50};
    sf::Font *p_font;
    sf::Color m_text_color;
public:
    std::string m_text;
    bool m_is_selected = false;

    MenuItem(sf::Font *font);
    void setFillColor(sf::Color color);

    sf::Vector2f getSize()const;

    virtual void handleEvent(sf::Event event) = 0;
    virtual void draw(sf::RenderWindow &window) = 0;
};

class ChangeStateItem : public MenuItem
{

public:
    ChangeStateItem( State::Context &context, StateStack *stack, States::ID destination, States::ID source = States::ID::None);
    virtual void handleEvent(sf::Event event) override;
    virtual void draw(sf::RenderWindow &window) override;

private:
    States::ID m_source;
    States::ID m_destination;
    StateStack *p_stack;

};

class ChangeKeyItem : public MenuItem
{
public:

    ChangeKeyItem(std::string command_name, PlayerControl command, State::Context &context);
    virtual void handleEvent(sf::Event event) override;
    virtual void draw(sf::RenderWindow &window)override;

private:

    bool m_is_changing_key = false;
    PlayerControl m_command;
    std::string m_command_name;
    std::string key_name;
    KeyBindings *p_bindings;


};

class Menu
{

public:
    Menu(sf::Font *font);

    void draw(sf::RenderWindow &window);
    void addItem(std::unique_ptr<MenuItem> item);
    void handleEvent(sf::Event event);

private:
    sf::Font *p_font;
    int selected_item_ind = 0;
    sf::Text m_text;
    std::vector<std::unique_ptr<MenuItem>> m_items;
};
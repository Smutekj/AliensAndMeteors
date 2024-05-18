#pragma once

#include "State.h"

#include <unordered_map>

#include "SFML/Graphics/Font.hpp"
#include "SFML/Graphics/Text.hpp"

#include "Menu.h"

class MenuState : public State
{

    enum MenuField : int
    {
        NEW_GAME = 0,
        SETTINGS,
        EXIT,
    };

    struct MenuFieldData
    {
        MenuField field;
        std::string text;
        States::ID destination;
        bool pop_current_state = false;
    };

    std::vector<MenuFieldData> fields;

    MenuField selected_field = MenuField::NEW_GAME;

    sf::Font menu_font;
    sf::Text field_text;

public:
    MenuState(StateStack &stack, Context &context);

    virtual ~MenuState() override;

    virtual void update(float dt) override;
    virtual void handleEvent(const sf::Event& event) override;
    virtual void draw() override;

    private:
    sf::Texture background_texture;
    Menu m_menu;
};

class EndScreenState : public State
{

    public:
        virtual ~EndScreenState() override;

        EndScreenState(StateStack &stack, Context &context);
        virtual void update(float dt) override;
        virtual void handleEvent(const sf::Event& event) override;
        virtual void draw() override;

    private:
    float m_timer;
    sf::Text m_goodbye_text;
  
};

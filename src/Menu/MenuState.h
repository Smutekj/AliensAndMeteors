#pragma once

#include "State.h"

#include <unordered_map>

#include "SFML/Graphics/Font.hpp"
#include "SFML/Graphics/Text.hpp"
#include "SFML/Graphics/RectangleShape.hpp"

#include "Menu.h"

class MenuState : public State
{

public:
    MenuState(StateStack &stack, Context &context);

    virtual ~MenuState() override;

    virtual void update(float dt) override;
    virtual void handleEvent(const sf::Event &event) override;
    virtual void draw() override;

private:
    sf::Texture m_background_texture;
    sf::RectangleShape m_background_rect;
    sf::Vector2f m_background_center;
    int m_background_animation_time = 0;
    Menu m_menu;
};


//! \brief State 
class EndScreenState : public State
{

public:
    virtual ~EndScreenState() override;

    EndScreenState(StateStack &stack, Context &context);
    virtual void update(float dt) override;
    virtual void handleEvent(const sf::Event &event) override;
    virtual void draw() override;

private:
    float m_timer = 600; //! after this time, the app quits
    sf::Text m_goodbye_text; 
};

class PlayerDiedState : public State
{

public:
    virtual ~PlayerDiedState() override;

    PlayerDiedState(StateStack &stack, Context &context);
    virtual void update(float dt) override;
    virtual void handleEvent(const sf::Event &event) override;
    virtual void draw() override;

private:
    std::string m_entered_name;
    sf::Text m_text;
    Menu m_menu;
};

class ScoreBoardState : public State
{

public:
    ScoreBoardState(StateStack &stack, Context &context);
    virtual ~ScoreBoardState() override;
    virtual void update(float dt) override;
    virtual void handleEvent(const sf::Event &event) override;
    virtual void draw() override;

private:
    void drawScoreLine(sf::RenderWindow &window,
                       std::string text1, std::string text2, float width, float y_position);
    sf::Text m_left_text;
    sf::Text m_right_text;
};
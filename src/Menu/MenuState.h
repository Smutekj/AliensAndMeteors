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
    sf::Texture background_texture;
    sf::RectangleShape background_rect;
    sf::Vector2f background_center;
    int background_animation_time = 0;
    Menu m_menu;
};

class EndScreenState : public State
{

public:
    virtual ~EndScreenState() override;

    EndScreenState(StateStack &stack, Context &context);
    virtual void update(float dt) override;
    virtual void handleEvent(const sf::Event &event) override;
    virtual void draw() override;

private:
    float m_timer = 600;
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
    bool m_is_entering_text = true;
    bool m_dash_is_visible = true;
    float m_dash_visibility_time = 0;
    float m_dash_visibility_cooldown = 60;
    std::string m_entered_name;
    sf::Text m_text;
};

class ScoreBoardState : public State
{

public:
    virtual ~ScoreBoardState() override;

    ScoreBoardState(StateStack &stack, Context &context);
    virtual void update(float dt) override;
    virtual void handleEvent(const sf::Event &event) override;
    virtual void draw() override;

private:
    void drawScoreLine(sf::RenderWindow &window,
                       std::string text1, std::string text2, float width, float y_position);
    sf::Text m_left_text;
    sf::Text m_right_text;
};
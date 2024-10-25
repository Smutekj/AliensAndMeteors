#pragma once

#include "State.h"
#include "Menu.h"

#include <unordered_map>

#include <Texture.h>
#include <Renderer.h>
#include <Sprite.h>

class MenuState : public State
{

public:
    MenuState(StateStack &stack, Context &context);

    virtual ~MenuState() override;

    virtual void update(float dt) override;
    virtual void handleEvent(const SDL_Event &event) override;
    virtual void draw() override;

private:
    Texture m_background_texture;
    Sprite m_background_rect;
    utils::Vector2f m_background_center;
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
    virtual void handleEvent(const SDL_Event &event) override;
    virtual void draw() override;

private:
    float m_timer = 600; //! after this time, the app quits
    Text m_goodbye_text; 
};

class PlayerDiedState : public State
{

public:
    virtual ~PlayerDiedState() override;

    PlayerDiedState(StateStack &stack, Context &context);
    virtual void update(float dt) override;
    virtual void handleEvent(const SDL_Event &event) override;
    virtual void draw() override;

private:
    std::string m_entered_name;
    Text m_text;
    Menu m_menu;
};

class ScoreBoardState : public State
{

public:
    ScoreBoardState(StateStack &stack, Context &context);
    virtual ~ScoreBoardState() override;
    virtual void update(float dt) override;
    virtual void handleEvent(const SDL_Event &event) override;
    virtual void draw() override;

private:
    void drawScoreLine(Renderer &window,
                       std::string text1, std::string text2, float width, float y_position);
    Text m_left_text;
    Text m_right_text;
};
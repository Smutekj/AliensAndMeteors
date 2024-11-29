#pragma once

#include <unordered_map>

#include "Menu.h"
#include "State.h"
#include "../Commands.h"



class SettingsState : public State
{
public:
    SettingsState(StateStack &stack, Context &context);
    virtual ~SettingsState() override {}
    virtual void update(float dt) override;
    virtual void handleEvent(const SDL_Event &event) override;
    virtual void draw() override;

private:
    Menu m_menu;
};

class KeyBindingState : public State
{
public:
    KeyBindingState(StateStack &stack, Context &context);
    virtual ~KeyBindingState() override {}
    virtual void update(float dt) override;
    virtual void handleEvent(const SDL_Event &event) override;
    virtual void draw() override;

private:
    Menu m_menu;
};

class GraphicsState : public State
{
public:
    GraphicsState(StateStack &stack, Context &context);
    virtual ~GraphicsState() override {}
    virtual void update(float dt) override;
    virtual void handleEvent(const SDL_Event &event) override;
    virtual void draw() override;

private:
    std::string m_screen_width_text;
    std::string m_screen_height_text;
    Menu m_menu;
};

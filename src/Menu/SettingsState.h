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

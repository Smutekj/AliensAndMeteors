#pragma once

#include "State.h"
#include "Menu.h"

class PauseState : public State
{

public:
    PauseState(StateStack &stack, Context &context);
    virtual ~PauseState() override {}
    virtual void update(float dt) override;
    virtual void handleEvent(const SDL_Event &event) override;
    virtual void draw() override;

private:
    Menu m_menu;
};

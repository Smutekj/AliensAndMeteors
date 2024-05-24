#pragma once


#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/View.hpp>

#include "State.h"
#include "Menu.h"

class PauseState : public State
{

public:
    PauseState(StateStack &stack, Context &context);
    virtual ~PauseState() override {}

    virtual void update(float dt) override;
    virtual void handleEvent(const sf::Event &event) override;
    virtual void draw() override;

private:
    Menu m_menu;
};

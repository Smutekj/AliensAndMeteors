#pragma once


#include <unordered_map>

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/View.hpp>

#include "Menu.h"
#include "State.h"
#include "../Commands.h"



class SettingsState : public State
{
public:
    SettingsState(StateStack &stack, Context &context);
    virtual ~SettingsState() override {}
    virtual void update(float dt) override;
    virtual void handleEvent(const sf::Event &event) override;
    virtual void draw() override;

private:
    Menu m_menu;
};

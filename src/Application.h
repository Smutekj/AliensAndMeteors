#pragma once

#include <sstream>

#include <SFML/Graphics.hpp>

#include "Menu/StateStack.h"
#include "Menu/ScoreBoard.h"

#include "ResourceIdentifiers.h"
#include "ResourceHolder.h"
#include "Commands.h"

class Application
{

public:
    Application(float fps = 60);

    void run();

private:
    std::unique_ptr<sf::RenderWindow> m_window;
    std::unique_ptr<StateStack> m_state_stack;

    ResourceHolder<sf::Texture, Textures::ID> m_textures;
    float m_dt;

    KeyBindings m_bindings;
    ScoreBoard m_score;
    sf::Font m_font;

    void registerStates();
};
#pragma once

#include "SFML/Graphics.hpp"

#include "StateStack.h"
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
    sf::Font font;

    void registerStates();
};
#pragma once

#include <unordered_map>

#include <SFML/Window/Keyboard.hpp>

enum class PlayerControl
{
    MOVE_FORWARD,
    MOVE_BACK,
    STEER_LEFT,
    STEER_RIGHT,
    THROW_BOMB,
    SHOOT_LASER,
    BOOST
};

class KeyBindings
{

public:
    KeyBindings();

    bool setBinding(PlayerControl command, sf::Keyboard::Key new_key);
    bool commandNotSet(PlayerControl command);
    void unsetKey(sf::Keyboard::Key new_key);
    void unsetCommand(PlayerControl command);
    sf::Keyboard::Key operator[](PlayerControl command) const;

private:
    std::unordered_map<PlayerControl, sf::Keyboard::Key> m_command_map;
    std::unordered_map<sf::Keyboard::Key, PlayerControl> m_key_map;
};

#include "Commands.h"

#include <cassert>

KeyBindings::KeyBindings()
{
    m_command_map[PlayerControl::MOVE_FORWARD] = sf::Keyboard::W;
    m_command_map[PlayerControl::MOVE_BACK] = sf::Keyboard::S;
    m_command_map[PlayerControl::STEER_LEFT] = sf::Keyboard::A;
    m_command_map[PlayerControl::STEER_RIGHT] = sf::Keyboard::D;
    m_command_map[PlayerControl::THROW_BOMB] = sf::Keyboard::B;
    m_command_map[PlayerControl::SHOOT_LASER] = sf::Keyboard::LControl;
    m_command_map[PlayerControl::BOOST] = sf::Keyboard::LShift;

    for (auto [command, key] : m_command_map)
    {
        m_key_map[key] = command;
    }
}

bool KeyBindings::setBinding(PlayerControl command, sf::Keyboard::Key new_key)
{
    auto old_key = m_command_map.at(command);
    m_key_map.erase(old_key);

    if (m_key_map.count(new_key) > 0) //! if key is already bound we switch the commands
    {
        auto old_command = m_key_map.at(new_key);
        m_command_map[old_command] = old_key;
        m_key_map[old_key] = old_command;
    }

    m_command_map[command] = new_key;
    m_key_map[new_key] = command;

    return true;
}

bool KeyBindings::commandNotSet(PlayerControl command)
{
    return m_command_map.count(command) == 0;
}

void KeyBindings::unsetKey(sf::Keyboard::Key new_key)
{
    assert(m_key_map.count(new_key) > 0);
    m_key_map.erase(new_key);
}
void KeyBindings::unsetCommand(PlayerControl command)
{
    auto old_key = m_command_map.at(command);
    m_key_map.erase(old_key);
}

sf::Keyboard::Key KeyBindings::operator[](PlayerControl command) const
{
    return m_command_map.at(command);
}
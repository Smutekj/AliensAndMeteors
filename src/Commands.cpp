#include "Commands.h"

#include <cassert>

KeyBindings::KeyBindings()
{
    m_command_map[PlayerControl::MOVE_FORWARD] = SDLK_w;
    m_command_map[PlayerControl::MOVE_BACK] = SDLK_s;
    m_command_map[PlayerControl::STEER_LEFT] = SDLK_a;
    m_command_map[PlayerControl::STEER_RIGHT] = SDLK_d;
    m_command_map[PlayerControl::THROW_BOMB] = SDLK_b;
    m_command_map[PlayerControl::SHOOT_LASER] = SDLK_LCTRL;
    m_command_map[PlayerControl::BOOST] = SDLK_LSHIFT;

    for (auto [command, key] : m_command_map)
    {
        m_key_map[key] = command;
    }
}

bool KeyBindings::setBinding(PlayerControl command, SDL_Keycode new_key)
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

void KeyBindings::unsetKey(SDL_Keycode new_key)
{
    assert(m_key_map.count(new_key) > 0);
    m_key_map.erase(new_key);
}
void KeyBindings::unsetCommand(PlayerControl command)
{
    auto old_key = m_command_map.at(command);
    m_key_map.erase(old_key);
}

SDL_Keycode KeyBindings::operator[](PlayerControl command) const
{
    return m_command_map.at(command);
}
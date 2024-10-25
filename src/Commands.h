#pragma once

#include <unordered_map>

#include <IncludesGl.h>

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

    bool setBinding(PlayerControl command, SDL_Keycode new_key);
    bool commandNotSet(PlayerControl command);
    void unsetKey(SDL_Keycode new_key);
    void unsetCommand(PlayerControl command);
    SDL_Keycode operator[](PlayerControl command) const;

private:
    std::unordered_map<PlayerControl, SDL_Keycode> m_command_map;
    std::unordered_map<SDL_Keycode, PlayerControl> m_key_map;
};

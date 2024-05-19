#pragma once

#include <unordered_map>

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
    std::unordered_map<PlayerControl, sf::Keyboard::Key> command_map;
    std::unordered_map<sf::Keyboard::Key, PlayerControl> key_map;

    // friend ChangeKeyItem;

public:
    KeyBindings()
    {
        command_map[PlayerControl::MOVE_FORWARD] = sf::Keyboard::W;
        command_map[PlayerControl::MOVE_BACK] = sf::Keyboard::S;
        command_map[PlayerControl::STEER_LEFT] = sf::Keyboard::A;
        command_map[PlayerControl::STEER_RIGHT] = sf::Keyboard::D;
        command_map[PlayerControl::THROW_BOMB] = sf::Keyboard::B;
        command_map[PlayerControl::SHOOT_LASER] = sf::Keyboard::LControl;
        command_map[PlayerControl::BOOST] = sf::Keyboard::LShift;

        for (auto [command, key] : command_map)
        {
            key_map[key] = command;
        }
    }

    bool setBinding(PlayerControl command, sf::Keyboard::Key new_key)
    {
        auto old_key = command_map.at(command);
        key_map.erase(old_key);

        if (key_map.count(new_key) > 0) //! if key is already bound we switch the commands
        {
            auto old_command = key_map.at(new_key);
            // assert(old_key == new_key);
            command_map[old_command] = old_key;
            key_map[old_key] = old_command;
            // assert(old_command != command);
            // return true;
        }

        command_map[command] = new_key;
        key_map[new_key] = command;

        return true;
    }

    bool commandNotSet(PlayerControl command)
    {
        return command_map.count(command) == 0;
    }

    void unsetKey(sf::Keyboard::Key new_key)
    {
        assert(key_map.count(new_key)>0);
        key_map.erase(new_key);
    }
    void unsetCommand(PlayerControl command)
    {
        auto old_key = command_map.at(command);
        // command_map.erase(command);
        key_map.erase(old_key);
    }

    sf::Keyboard::Key operator[](PlayerControl command) const
    {
        return command_map.at(command);
    }
};

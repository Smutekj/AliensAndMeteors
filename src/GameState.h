#pragma once

#include "State.h"
#include <memory>
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
        if (key_map.count(new_key) > 0)
        {
            return false;
        }

        command_map[command] = new_key;
        key_map[new_key] = command;

        return true;
    }

    sf::Keyboard::Key operator[](PlayerControl command) const
    {
        return command_map.at(command);
    }
};

class Game;
class StateStack;

class GameState : public State
{

public:
    GameState(StateStack &stack, State::Context context);

    virtual ~GameState() override;

    virtual void update(float dt) override;

    virtual void draw() override;

    virtual void handleEvent(const sf::Event &event) override;

private:
    std::unique_ptr<Game> mp_game;

    KeyBindings bindings;
};
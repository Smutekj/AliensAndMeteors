#pragma once

#include "State.h"
#include <memory>
#include <unordered_map>

#include "SettingsState.h"


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
};
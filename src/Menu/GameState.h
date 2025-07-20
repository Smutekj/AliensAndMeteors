#pragma once

#include <memory>
#include <deque>
#include <unordered_map>

#include "State.h"
#include "SettingsState.h"
#include "UIDocument.h"

class StateStack;
class Game;

class GameState : public State
{

public:
    GameState(StateStack &stack, State::Context context);

    virtual ~GameState() override;
    virtual void update(float dt) override;
    virtual void draw() override;
    virtual void handleEvent(const SDL_Event &event) override;

private:
    std::shared_ptr<Game> mp_game;
};

struct ShopItems
{

    int max_fuel = 100;
    int max_speed = 100;
    int max_accel = 100;
    int max_booster = 100;
};

struct ShopItem
{

    int value = 0;
    int max_value = 100;
    int price = 1;
};

struct ShopEntry
{
    UIElement* ui_node = nullptr; 
    std::string name;
    int price = 10;
    int level = 0;
    int max_level = 10;

    bool buy(int& player_money)
    {
        if(player_money < price || level == max_level)
        {
            return false;
        }

        player_money -= price;
        price += 10;
        level += 1;
        return true;
    }
    bool sell(int& player_money)
    {
        if(level == 0)
        {
            return false;
        }

        player_money += price / 2;
        price -= 10;
        level -= 1;
        return true;
    }
};

struct PlayerEntity;

class ShopState : public State
{

public:
    ShopState(StateStack &stack, State::Context context);

    virtual ~ShopState() override;
    virtual void update(float dt) override;
    virtual void draw() override;
    virtual void handleEvent(const SDL_Event &event) override;

    void initButtons();

private:
    std::unordered_map<std::string, ShopEntry> m_items;
    
    std::shared_ptr<Game> mp_game;

    UIDocument document;
};
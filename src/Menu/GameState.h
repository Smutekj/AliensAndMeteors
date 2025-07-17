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

struct ItemUIElement
{
    Rect<int> bounding_box = {};
    std::string name = "Placeholder";
    std::string sprite_name = "Fuel";
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

private:
    std::unordered_map<std::string, ShopItem> m_items;

    std::vector<ItemUIElement> m_ui_elements;
    int n_elements_per_row = 3;


    std::shared_ptr<Game> mp_game;

    UIDocument document;
};
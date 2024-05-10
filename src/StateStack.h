#pragma once

#include "State.h"

#include <SFML/System/NonCopyable.hpp>
#include <SFML/System/Time.hpp>

#include <vector>
#include <utility>
#include <functional>
#include <map>


namespace sf
{
    class Event;
    class RenderWindow;
}


//! STOLEN AND ADAPTED FROM SFML-BOOK!!! Not my code :(
//! https://github.com/SFML/SFML-Game-Development-Book/blob/master/06_Menus
class StateStack : private sf::NonCopyable
{
public:
    enum Action
    {
        Push,
        Pop,
        Clear,
    };

public:
    explicit StateStack(State::Context context);

    template <typename T>
    void registerState(States::ID stateID);

    void update(float dt);
    void draw();
    void handleEvent(const sf::Event &event);

    void pushState(States::ID stateID);
    void popState();
    void clearStates();

    bool isEmpty() const;

private:
    std::unique_ptr<State> createState(States::ID stateID);
    void applyPendingChanges();

private:
    struct PendingChange
    {
        explicit PendingChange(Action action, States::ID stateID = States::None);

        Action action;
        States::ID stateID;
    };

private:
    std::vector<std::unique_ptr<State>> mStack;
    std::vector<PendingChange> mPendingList;

    State::Context mContext;
    std::map<States::ID, std::function<std::unique_ptr<State>()>> mFactories;
};

template <typename T>
void StateStack::registerState(States::ID stateID)
{
    mFactories[stateID] = [this]()
    {
        return std::make_unique<T>(*this, mContext);
    };
}

#pragma once

#include <SFML/System/NonCopyable.hpp>
#include <SFML/System/Time.hpp>

#include <vector>
#include <utility>
#include <functional>
#include <map>

#include "State.h"

namespace sf
{
    class Event;
    class RenderWindow;
}


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
    
    template <class T, class... Args>
    void pushState2(Args...);
    
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

template <class T, class... Args>
void StateStack::pushState2(Args... arguments)
{
    mStack.push_back(std::make_unique<T>(*this, mContext, arguments...));
}



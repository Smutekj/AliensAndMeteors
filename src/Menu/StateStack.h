#pragma once

#include <SFML/System/NonCopyable.hpp>
#include <SFML/System/Time.hpp>

#include <queue>
#include <stack>
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
    std::stack<std::unique_ptr<State>> m_stack;
    std::queue<PendingChange> m_pending_changes;

    State::Context m_context;
    std::map<States::ID, std::function<std::unique_ptr<State>()>> m_factories;
};

template <typename T>
void StateStack::registerState(States::ID stateID)
{
    m_factories[stateID] = [this]()
    {
        return std::make_unique<T>(*this, m_context);
    };
}

template <class T, class... Args>
void StateStack::pushState2(Args... arguments)
{
    m_stack.push(std::make_unique<T>(*this, m_context, arguments...));
}



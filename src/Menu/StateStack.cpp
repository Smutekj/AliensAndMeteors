#include "StateStack.h"

#include <cassert>
#include <Renderer.h>

StateStack::StateStack(State::Context context)
    : m_stack(), m_pending_changes(), m_context(context), m_factories()
{
}

void StateStack::update(float dt)
{

    if (m_stack.size() > 0)
    {
        m_stack.top()->update(dt);
    }
    applyPendingChanges();

}

void StateStack::draw()
{
    m_context.window->clear({0,0,0,255});
    if (m_stack.size() > 0)
    {
        m_stack.top()->draw();
    
    }
    m_context.window->drawAll();
};

void StateStack::pushState(States::ID stateID)
{
    if (stateID != States::ID::None)
    {
        m_pending_changes.push(PendingChange(Push, stateID));
    }
}

void StateStack::popState()
{
    m_pending_changes.push(PendingChange(Pop));
}

void StateStack::clearStates()
{
    m_pending_changes.push(PendingChange(Clear));
}

bool StateStack::isEmpty() const
{
    return m_stack.empty();
}

std::unique_ptr<State> StateStack::createState(States::ID stateID)
{
    return m_factories.at(stateID)(); //! calls the factory method
}

void StateStack::applyPendingChanges()
{
    while (!m_pending_changes.empty())
    {
        auto &[action, state] = m_pending_changes.front();
        switch (action)
        {
        case Push:
            m_stack.push(createState(state));
            break;

        case Pop:
            m_stack.pop();
            break;

        case Clear:
            m_stack = {};
            break;
        }
        m_pending_changes.pop();
    }
}

StateStack::PendingChange::PendingChange(Action action, States::ID stateID)
    : action(action), stateID(stateID)
{
}

void StateStack::handleEvent(const SDL_Event &event)
{

    if (m_stack.size() > 0)
    {
        m_stack.top()->handleEvent(event);
    }
}
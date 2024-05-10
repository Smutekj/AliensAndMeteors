#include "StateStack.h"

#include <cassert>

StateStack::StateStack(State::Context context)
    : mStack(), mPendingList(), mContext(context), mFactories()
{
}

void StateStack::update(float dt)
{
    // Iterate from top to bottom, stop as soon as update() returns false
    for (int i = mStack.size() - 1; i >= 0; --i)
    {
        mStack.at(i)->update(dt);
        if (mStack.at(i)->isFinalState())
        {
            break;
        }
    }

    applyPendingChanges();
}

void StateStack::draw()
{
    // Draw all active states from bottom to top
    for (int i = mStack.size() - 1; i >= 0; --i)
    {
        mStack.at(i)->draw();
        if (mStack.at(i)->isFinalState())
        {
            break;
        }
    }
}

void StateStack::pushState(States::ID stateID)
{
    if (stateID != States::ID::None)
    {
        mPendingList.push_back(PendingChange(Push, stateID));
    }
}

void StateStack::popState()
{
    mPendingList.push_back(PendingChange(Pop));
}

void StateStack::clearStates()
{
    mPendingList.push_back(PendingChange(Clear));
}

bool StateStack::isEmpty() const
{
    return mStack.empty();
}

std::unique_ptr<State> StateStack::createState(States::ID stateID)
{
    auto found = mFactories.find(stateID);
    assert(found != mFactories.end());

    return found->second();
}

void StateStack::applyPendingChanges()
{
    for (auto &change : mPendingList)
    {
        switch (change.action)
        {
        case Push:
            mStack.push_back(createState(change.stateID));
            break;

        case Pop:
            mStack.pop_back();
            break;

        case Clear:
            mStack.clear();
            break;
        }
    }

    mPendingList.clear();
}

StateStack::PendingChange::PendingChange(Action action, States::ID stateID)
    : action(action), stateID(stateID)
{
}

void StateStack::handleEvent(const sf::Event &event)
{
    for (int i = mStack.size() - 1; i >= 0; --i)
    {
        mStack.at(i)->handleEvent(event);
    }
}
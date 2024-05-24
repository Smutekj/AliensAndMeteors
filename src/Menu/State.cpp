#include "State.h"
#include "StateStack.h"

State::State(StateStack &stack, Context context)
	: m_stack(&stack),
	  m_context(context)
{
}
void State::requestStackPush(States::ID stateID)
{

	m_stack->pushState(stateID);
	
}
void State::requestStackPop()
{
	m_stack->popState();
}

void State::requestStateClear()
{
	m_stack->clearStates();
}

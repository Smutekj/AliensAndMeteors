#include "PauseState.h"
#include "State.h"

#include "SFML/Graphics/RenderWindow.hpp"

PauseState::PauseState(StateStack &stack, Context &context) : State(stack, context), m_menu(context.font)
{

    auto resume = std::make_unique<ChangeStateItem>(context, &stack, States::ID::None, States::ID::None);
    resume->m_text = "Resume";
    auto settings = std::make_unique<ChangeStateItem>(context, &stack, States::ID::Settings, States::ID::Pause);
    auto back_to_menu = std::make_unique<ChangeStateItem>(context, &stack, States::ID::Menu, States::ID::None);
    resume->m_text = "Quit Game";
    
    m_menu.addItem(std::move(resume));
    m_menu.addItem(std::move(settings));
    m_menu.addItem(std::move(back_to_menu));
}

void PauseState::update(float dt)
{
}

void PauseState::handleEvent(const sf::Event &event)
{
    m_menu.handleEvent(event);
}

void PauseState::draw()
{
    auto &window = *getContext().window;
    m_menu.draw(window);
}

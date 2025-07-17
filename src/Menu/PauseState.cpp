#include "PauseState.h"
#include "State.h"

PauseState::PauseState(StateStack &stack, Context &context) : State(stack, context), m_menu(context.font)
{

    auto resume = std::make_unique<ChangeStateItem>(context, &stack, States::ID::None, States::ID::None, "Resume");
    auto settings = std::make_unique<ChangeStateItem>(context, &stack, States::ID::Settings, States::ID::Pause);
    auto back_to_menu = std::make_unique<ChangeStateItem>(context, &stack,
                                                          States::ID::Player_Died, States::ID::None, "Quit Game");
    auto shop = std::make_unique<ChangeStateItem>(context, &stack,
                                                          States::ID::Shop, States::ID::None, "Shop");

    m_menu.addItem(std::move(resume));
    m_menu.addItem(std::move(shop));
    m_menu.addItem(std::move(settings));
    m_menu.addItem(std::move(back_to_menu));
}

void PauseState::update(float dt)
{
}

void PauseState::handleEvent(const SDL_Event &event)
{
    m_menu.handleEvent(event);
}

void PauseState::draw()
{
    auto &window = *m_context.window;
    m_menu.draw(window);
}

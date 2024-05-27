#include "SettingsState.h"

#include <SFML/Graphics/RenderWindow.hpp>

#include "../Utils/magic_enum.hpp"
#include "../Utils/magic_enum_utility.hpp"

#include "State.h"

SettingsState::SettingsState(StateStack &stack, Context &context) : State(stack, context), m_menu(context.font)
{

    auto change_key_1 = std::make_unique<ChangeKeyItem>("Forward", PlayerControl::MOVE_FORWARD, context);
    auto change_key_2 = std::make_unique<ChangeKeyItem>("Back", PlayerControl::MOVE_BACK, context);
    auto change_key_3 = std::make_unique<ChangeKeyItem>("Steer right", PlayerControl::STEER_LEFT, context);
    auto change_key_4 = std::make_unique<ChangeKeyItem>("Steer left", PlayerControl::STEER_RIGHT, context);
    auto change_key_5 = std::make_unique<ChangeKeyItem>("Boost", PlayerControl::BOOST, context);
    auto change_key_6 = std::make_unique<ChangeKeyItem>("Throw Bomb", PlayerControl::THROW_BOMB, context);
    auto change_key_7 = std::make_unique<ChangeKeyItem>("Shoot Laser", PlayerControl::SHOOT_LASER, context);
    m_menu.addItem(std::move(change_key_1));
    m_menu.addItem(std::move(change_key_2));
    m_menu.addItem(std::move(change_key_3));
    m_menu.addItem(std::move(change_key_4));
    m_menu.addItem(std::move(change_key_5));
    m_menu.addItem(std::move(change_key_6));
    m_menu.addItem(std::move(change_key_7));

    auto back = std::make_unique<ChangeStateItem>(context, m_stack, States::ID::None, States::ID::None, "Back");
    m_menu.addItem(std::move(back));

}

void SettingsState::update(float dt)
{
}

void SettingsState::handleEvent(const sf::Event &event)
{
    m_menu.handleEvent(event);
}

void SettingsState::draw()
{
    auto &window = *m_context.window;
    m_menu.draw(window);
}



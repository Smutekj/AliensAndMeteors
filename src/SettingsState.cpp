#include "SettingsState.h"
#include "State.h"

#include "SFML/Graphics/RenderWindow.hpp"

#include "Utils/magic_enum.hpp"
#include "Utils/magic_enum_utility.hpp"

SettingsState::SettingsState(StateStack &stack, Context &context) : State(stack, context), m_menu(context.font)
{
    m_is_final_state = true;

    int num_commands = static_cast<int>(magic_enum::enum_count<PlayerControl>());
    magic_enum::enum_for_each<PlayerControl>([&](PlayerControl control)
        {
            std::string control_name = static_cast<std::string>(magic_enum::enum_name(control));

            auto change_key_1 = std::make_unique<ChangeKeyItem>(control_name, control, context.bindings);
            m_menu.addItem(std::move(change_key_1));
        });

    auto back = std::make_unique<ChangeStateItem>(m_stack, States::ID::Menu);
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

    auto &window = *getContext().window;
    m_menu.draw(window);
}



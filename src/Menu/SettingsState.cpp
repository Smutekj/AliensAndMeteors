#include "SettingsState.h"
#include "State.h"
#include "Window.h"

#include "../../external/magic_enum/magic_enum.hpp"
#include "../../external/magic_enum/magic_enum_utility.hpp"

SettingsState::SettingsState(StateStack &stack, Context &context) : State(stack, context), m_menu(context.font)
{

    auto key_bindings = std::make_unique<ChangeStateItem>(context, m_stack,
                                                          States::ID::KeyBindings, States::ID::None, "Key Bindings");
    m_menu.addItem(std::move(key_bindings));

    auto graphics = std::make_unique<ChangeStateItem>(context, m_stack,
                                                      States::ID::Graphics, States::ID::None, "Graphics");
    m_menu.addItem(std::move(graphics));

    auto back = std::make_unique<ChangeStateItem>(context, m_stack, States::ID::None, States::ID::None, "Back");
    m_menu.addItem(std::move(back));
}

void SettingsState::update(float dt)
{
}

void SettingsState::handleEvent(const SDL_Event &event)
{
    m_menu.handleEvent(event);
}

void SettingsState::draw()
{
    auto &window = *m_context.window;
    m_menu.draw(window);
}

KeyBindingState::KeyBindingState(StateStack &stack, Context &context) : State(stack, context), m_menu(context.font)
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

    // auto resize_window = std::make_unique<ResizeWindowItem>("Change Window Size", PlayerControl::SHOOT_LASER, context);
    // m_menu.addItem(std::move(resize_window));

    auto back = std::make_unique<ChangeStateItem>(context, m_stack, States::ID::Settings, States::ID::None, "Back");
    m_menu.addItem(std::move(back));
}

void KeyBindingState::update(float dt)
{
}

void KeyBindingState::handleEvent(const SDL_Event &event)
{
    m_menu.handleEvent(event);
}

void KeyBindingState::draw()
{
    auto &window = *m_context.window;
    m_menu.draw(window);
}

bool isWindowed(Window &window)
{
    Uint32 windowFlags = SDL_GetWindowFlags(window.getHandle());
    if ((windowFlags & SDL_WINDOW_FULLSCREEN_DESKTOP) == SDL_WINDOW_FULLSCREEN_DESKTOP)
    {
        return false;
    }
    else if (windowFlags & SDL_WINDOW_FULLSCREEN)
    {
        return false;
    } // (windowFlags & SDL_WINDOW_FULLSCREEN) == 0
    return true;
}

GraphicsState::GraphicsState(StateStack &stack, Context &context) : State(stack, context), m_menu(context.font)
{
    auto toggle_fs = [this, window = context.window_handle]()
    {
        if(isWindowed(*window))
        {
            SDL_SetWindowFullscreen(window->getHandle(), SDL_WINDOW_FULLSCREEN_DESKTOP);
        }else{
            SDL_SetWindowFullscreen(window->getHandle(), 0);
        }
    };

    m_screen_width_text = std::to_string(context.window->getTargetSize().x);
    m_screen_height_text = std::to_string(context.window->getTargetSize().y);

    auto fullscreen = std::make_unique<CallBackItem>(context, "Toggle Fullscreen", toggle_fs);
    m_menu.addItem(std::move(fullscreen));

    auto enter_width = std::make_unique<EnterNumberItem>(context, m_screen_width_text, "Screen Width:");
    m_menu.addItem(std::move(enter_width));
    auto enter_height = std::make_unique<EnterNumberItem>(context, m_screen_height_text, "Screen Height:");
    m_menu.addItem(std::move(enter_height));
    
    auto toggle_screen_resize = [this, window = context.window_handle]()
    {   
        int new_screen_width = std::stoi(m_screen_width_text);
        int new_screen_height = std::stoi(m_screen_height_text);
        window->setSize(new_screen_width, new_screen_height);
    };
    auto change_screen_size = std::make_unique<CallBackItem>(context, "Change Window Size", toggle_screen_resize);
    m_menu.addItem(std::move(change_screen_size));

    auto back = std::make_unique<ChangeStateItem>(context, m_stack, States::ID::None, States::ID::None, "Back");
    m_menu.addItem(std::move(back));
}

void GraphicsState::update(float dt)
{
}

void GraphicsState::handleEvent(const SDL_Event &event)
{
    m_menu.handleEvent(event);
}

void GraphicsState::draw()
{
    auto &window = *m_context.window;
    m_menu.draw(window);
}

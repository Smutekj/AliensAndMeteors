#pragma once

#include "State.h"
#include "../Commands.h"

#include <Utils/Vector2.h>
#include <Transform.h>
#include <Renderer.h>

#include <functional>

class MenuItem : public Transform
{

public:
    bool m_is_selected = false;
    MenuItem(Font *font);
    void setFillColor(ColorByte color);
    utils::Vector2f getSize() const;
    void setString(std::string new_string);
    virtual void handleEvent(SDL_Event event) = 0;
    virtual void draw(Renderer &window) = 0;

protected:
    utils::Vector2f m_item_size = {300, 50};
    Font *p_font;
    ColorByte m_text_color;
    Text m_text;
};

//! \class item that pushes \memberof m_destination state on the stack,
//!         if the \memberof m_source is None, the current state is popped
//!         (therefore it won't be possible to return to him)
class ChangeStateItem : public MenuItem
{

public:
    ChangeStateItem(State::Context &context, StateStack *stack,
                    States::ID destination, States::ID source = States::ID::None, std::string button_text = "");
    virtual void handleEvent(SDL_Event event) override;
    virtual void draw(Renderer &window) override;

private:
    States::ID m_source;
    States::ID m_destination;
    StateStack *p_stack;
};

class EnterTextItem : public MenuItem
{

public:
    EnterTextItem(State::Context &context, std::string &text, std::string left_text);
    virtual void handleEvent(SDL_Event event) override;
    virtual void draw(Renderer &window) override;

private:
    bool m_is_changing_key = false;
    std::string m_left_text = "Enter IP Address:";
    std::string &m_entered_text;

private:
    bool isLetter(u_int32_t code);
};

class CallBackItem : public MenuItem
{

public:
    CallBackItem(State::Context &context, std::function<void()> callback);
    virtual void handleEvent(SDL_Event event) override;
    virtual void draw(Renderer &window) override;

private:
    std::function<void()> m_callback;
};

//! \class item that changes key_bindings
//!         if existing key is selected, the command bound to the key is unbound
//! \todo   make it impossible to return back when a some key is unbound
class ChangeKeyItem : public MenuItem
{
public:
    ChangeKeyItem(std::string command_name, PlayerControl command, State::Context &context);
    virtual void handleEvent(SDL_Event event) override;
    virtual void draw(Renderer &window) override;

private:
    bool m_is_changing_key = false;
    PlayerControl m_command;
    std::string m_command_name;
    KeyBindings *p_bindings;
};

//! \class contains a list of MenuItems which can be navigated by up/down arrows
class Menu
{

public:
    Menu(Font *font);

    void draw(Renderer &window);
    void addItem(std::unique_ptr<MenuItem> item);
    void handleEvent(SDL_Event event);
    void setYPos(float y_pos);

    static void centerTextInWindow(const Renderer &window, Text &m_text, float y_coord);

private:
    Font *p_font;
    int m_selected_item_ind = 0;
    float m_y_pos = 100.f;
    Text m_text;
    std::vector<std::unique_ptr<MenuItem>> m_items;
};

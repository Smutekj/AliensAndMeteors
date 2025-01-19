#include "Menu.h"

#include "../../external/magic_enum/magic_enum.hpp"
#include "../../external/magic_enum/magic_enum_utility.hpp"

#include "StateStack.h"

MenuItem::MenuItem(Font *font) : p_font(font)
{
    m_text.setFont(font);
}

void MenuItem::setFillColor(ColorByte color)
{
    m_text_color = color;
}

void MenuItem::setString(std::string new_string)
{
    m_text.setText(new_string);
}

utils::Vector2f MenuItem::getSize() const
{
    return m_item_size;
}

Menu::Menu(Font *font) : p_font(font)
{
    m_text.setFont(p_font);
}

void Menu::setYPos(float y_pos)
{
    m_y_pos = y_pos;
}

void Menu::draw(Renderer &window)
{
    auto old_view = window.m_view;
    auto window_size = window.getTargetSize();
    window.m_view = window.getDefaultView();

    utils::Vector2f size = {static_cast<float>(window_size.x), static_cast<float>(window_size.y)};

    float field_y_pos = 0.9 * size.y - m_y_pos;
    m_text.setColor({255, 0, 0, 255});

    for (int item_ind = 0; item_ind < m_items.size(); item_ind++)
    {
        auto &field = m_items.at(item_ind);
        if (item_ind == m_selected_item_ind)
        {
            field->setScale(1.1f, 1.1f);
            field->setFillColor({255, 0, 0, 255});
        }
        else
        {
            field->setFillColor({255, 255, 255, 255});
            field->setScale(1.f, 1.f);
        }

        field->setPosition(window_size.x / 2.f, field_y_pos);
        field->draw(window);

        field_y_pos -= field->getSize().y;
    }
    window.drawAll();
    window.m_view = (old_view);
}

void Menu::addItem(std::unique_ptr<MenuItem> item)
{
    m_items.push_back(std::move(item));
}

void Menu::handleEvent(SDL_Event event)
{
    if (event.type == SDL_KEYUP)
    {
        if (event.key.keysym.sym == SDLK_UP) //! select item above
        {
            m_items.at(m_selected_item_ind)->m_is_selected = false;
            m_selected_item_ind = (m_selected_item_ind - 1 + m_items.size()) % m_items.size();
            m_items.at(m_selected_item_ind)->m_is_selected = true;
        }
        else if (event.key.keysym.sym == SDLK_DOWN) //! select item below
        {
            m_items.at(m_selected_item_ind)->m_is_selected = false;
            m_selected_item_ind = (m_selected_item_ind + 1) % m_items.size();
            m_items.at(m_selected_item_ind)->m_is_selected = true;
        }
    }

    m_items.at(m_selected_item_ind)->handleEvent(event);
}

ChangeStateItem::ChangeStateItem(State::Context &context, StateStack *stack,
                                 States::ID destination, States::ID source, std::string button_text)
    : m_source(source), m_destination(destination), p_stack(stack), MenuItem(context.font)
{

    if (button_text.empty())
    {
        std::string destination_name = static_cast<std::string>(magic_enum::enum_name(m_destination));
        m_text.setText(destination_name);
    }
    else
    {
        m_text.setText(button_text);
    }
}

void ChangeStateItem::handleEvent(SDL_Event event)
{
    if (event.type == SDL_KEYUP)
    {
        bool key_is_enter = event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_KP_ENTER;
        if (key_is_enter)
        {
            if (m_source == States::ID::None) //! if there is no source we don't want to return
            {
                p_stack->popState();
                std::cout << "State popped!" << std::endl;
            }
            p_stack->pushState(m_destination);
        }
    }
}

void ChangeStateItem::draw(Renderer &window)
{
    Menu::centerTextInWindow(window, m_text, getPosition().y);
    m_text.setColor(m_text_color);

    window.drawText(m_text);
}

ChangeKeyItem::ChangeKeyItem(std::string command_name, PlayerControl command, State::Context &context)
    : m_command(command), m_command_name(command_name), p_bindings(context.bindings), MenuItem(context.font)
{
    m_item_size.x = 200.f;
    m_item_size.y = 50.f;
}

void ChangeKeyItem::handleEvent(SDL_Event event)
{

    if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_RETURN)
    {

        if (!m_is_changing_key)
        {
            m_is_changing_key = true;
            // p_bindings->unsetCommand(m_command);
        }
    }
    else if (m_is_changing_key)
    {
        if (p_bindings->setBinding(m_command, (SDL_KeyCode)event.key.keysym.sym)) //! if we succesfully changed key
        {
            m_is_changing_key = false;
        }
    }
}

void ChangeKeyItem::draw(Renderer &window)
{


    
    Text left_text;
    left_text.setFont(p_font);
    utils::Vector2f left_text_pos = {
        getPosition().x - m_item_size.x,
        getPosition().y};
    left_text.setText(m_command_name);
    left_text.setPosition(left_text_pos);
    left_text.setColor(m_text_color);
    Text right_text;
    right_text.setFont(p_font);

    if (m_is_changing_key)
    {
        right_text.setText("...");
    }
    else
    {
        right_text.setText(p_bindings->keyName(m_command));
    }

    right_text.setColor(m_text_color);

    utils::Vector2f right_text_pos = {
        getPosition().x + m_item_size.x - 0.f,
        getPosition().y};
    right_text.setPosition(right_text_pos);

    window.drawText(left_text);
    window.drawText(right_text);
}

void Menu::centerTextInWindow(const Renderer &window, Text &m_text, float y_coord)
{
    auto window_size = window.getTargetSize();
    m_text.centerAround({window_size.x / 2.f, y_coord});
}

EnterTextItem::EnterTextItem(State::Context &context, std::string &text, std::string left_text)
    : MenuItem(context.font), m_entered_text(text), m_left_text(left_text)
{
}

void EnterTextItem::handleEvent(SDL_Event event)
{

    if (event.type == SDL_KEYUP || event.type == SDL_KEYDOWN)
    {

        if (event.key.keysym.sym == SDLK_BACKSPACE && m_entered_text.size() > 0)
        {
            m_entered_text.pop_back();
        }
        if (event.key.keysym.sym == SDLK_PERIOD)
        {
            m_entered_text.push_back('.');
        }
    }
    else if (event.type == SDL_TEXTINPUT)
    {
        m_entered_text += event.text.text;
    }
}

void EnterTextItem::draw(Renderer &window)
{
    auto window_size = window.getTargetSize();

    Text left_text;
    left_text.setFont(p_font);
    utils::Vector2f left_text_pos = {
        getPosition().x - window_size.x / 4.f,
        getPosition().y};
    left_text.setText(m_left_text);
    left_text.centerAround(left_text_pos);
    left_text.setColor(m_text_color);

    Text right_text;
    right_text.setFont(p_font);
    utils::Vector2f right_text_pos = {
        getPosition().x + window_size.x / 4.f,
        getPosition().y};
    right_text.setText(m_entered_text);
    right_text.centerAround(right_text_pos);

    window.drawText(left_text);
    window.drawText(right_text);
}

bool EnterTextItem::isLetter(uint32_t code)
{
    return (code >= 48 && code <= 57) || (code >= 65 && code <= 90) || (code >= 97 && code <= 122);
}

CallBackItem::CallBackItem(State::Context &context, std::string item_name, std::function<void()> callback)
    : MenuItem(context.font), m_callback(callback)
{
    m_text.setFont(context.font);
    m_text.setText(item_name);
}

void CallBackItem::handleEvent(SDL_Event event)
{
    if (event.type == SDL_KEYUP)
    {
        bool enter_hit = event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_KP_ENTER;
        if (enter_hit)
        {
            m_callback();
        }
    }
}

void CallBackItem::draw(Renderer &window)
{
    m_text.centerAround({window.getTargetSize().x / 2.f, getPosition().y});
    m_text.setColor(m_text_color);
    window.drawText(m_text);
}

EnterNumberItem::EnterNumberItem(State::Context &context, std::string &text, std::string left_text)
    : MenuItem(context.font), m_entered_text(text), m_left_text(left_text)
{
}

void EnterNumberItem::handleEvent(SDL_Event event)
{

    if (event.type == SDL_KEYUP)
    {
        bool key_is_enter = event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_KP_ENTER;

        if (key_is_enter)
        {
            m_is_changing_key = !m_is_changing_key;
        }
        else
        {
            if (event.key.keysym.sym == SDLK_BACKSPACE && m_entered_text.size() > 0)
            {
                m_entered_text.pop_back();
            }
        }
    }
    else if (event.type == SDL_TEXTINPUT && m_is_changing_key)
    {
        if (event.text.text[0] >= '0' && event.text.text[0] <= '9')
        {
            m_entered_text = m_entered_text + event.text.text[0];
        }
    }

    if (event.key.keysym.sym == SDLK_ESCAPE)
    {
        m_is_changing_key = false;
    }
}

void EnterNumberItem::draw(Renderer &window)
{

    Text left_text;
    left_text.setFont(m_text.getFont());
    utils::Vector2f left_text_pos = {
        getPosition().x - m_item_size.x,
        getPosition().y};
    left_text.setPosition(left_text_pos);
    left_text.setColor(m_text_color);
    left_text.setText(m_left_text);

    Text right_text;
    right_text.setFont(m_text.getFont());
    right_text.setText(m_entered_text);
    utils::Vector2f right_text_pos = {
        getPosition().x + m_item_size.x - right_text.getBoundingBox().width,
        getPosition().y};
    right_text.setPosition(right_text_pos);

    window.drawText(left_text);
    window.drawText(right_text);
}

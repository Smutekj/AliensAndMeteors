#pragma once

#include "State.h"

#include <unordered_map>

#include "SFML/Graphics/Font.hpp"
#include "SFML/Graphics/Text.hpp"
#include "SFML/Graphics/View.hpp"



class SettingsState : public State
{

    enum MenuField : int
    {
        CHANGE_MOVE_FORWARD,
        CHANGE_MOVE_BACK,
        CHANGE_STEER_LEFT,
        CHANGE_STEER_RIGHT,
        CHANGE_THROW_BOMB,
        CHANGE_SHOOT_LASER,
        CHANGE_BOOST,
        RESUME
    };

    struct MenuFieldData
    {
        MenuField field;
        std::string text;
        States::ID destination;
        sf::Keyboard::Key bound_key;
    };

    std::unordered_map<MenuField, MenuFieldData> field2data;
    std::vector<MenuFieldData> fields;

    MenuField selected_field = MenuField::RESUME;

    sf::Font menu_font;
    sf::Text field_text;


public:
    SettingsState(StateStack &stack, Context &context);

    virtual ~SettingsState() override {}

    virtual void update(float dt) override;
    virtual void handleEvent(const sf::Event& event) override;

    virtual void draw() override;

    private:
    void moveSelectionUp(){
        // if(selected_field == MenuField::RESUME){
        //     selected_field = MenuField::EXIT;
        // }else{
        //     selected_field = static_cast<MenuField>(selected_field -1);
        // }
    }
    void moveSelectionDown(){
        // if(selected_field == MenuField::EXIT){
        //     selected_field = MenuField::RESUME;
        // }else{
        //     selected_field = static_cast<MenuField>(selected_field + 1 );
        // }
    }

    private:
        // KeyBinding& bindings;

        sf::View old_view;
};

#pragma once

#include "State.h"

#include <unordered_map>

#include "SFML/Graphics/Font.hpp"
#include "SFML/Graphics/Text.hpp"
#include "SFML/Graphics/View.hpp"

class PauseState : public State
{

    enum MenuField : int
    {
        RESUME = 0,
        SETTINGS,
        EXIT,
    };

    struct MenuFieldData
    {
        MenuField field;
        std::string text;
        States::ID destination;
        bool pop_current_state = false;
    };

    std::unordered_map<MenuField, MenuFieldData> field2data;
    std::vector<MenuFieldData> fields;

    MenuField selected_field = MenuField::RESUME;

    sf::Font menu_font;
    sf::Text field_text;

public:
    PauseState(StateStack &stack, Context &context);

    virtual ~PauseState() override {}

    virtual void update(float dt) override;
    virtual void handleEvent(const sf::Event& event) override;

    virtual void draw() override;

    private:
    void moveSelectionUp(){
        if(selected_field == MenuField::RESUME){
            selected_field = MenuField::EXIT;
        }else{
            selected_field = static_cast<MenuField>(selected_field -1);
        }
    }
    void moveSelectionDown(){
        if(selected_field == MenuField::EXIT){
            selected_field = MenuField::RESUME;
        }else{
            selected_field = static_cast<MenuField>(selected_field + 1 );
        }
    }

    private:
        sf::View old_view;
};

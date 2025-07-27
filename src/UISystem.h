#pragma once

#include "Menu/UIDocument.h"
#include "PostOffice.h"

class UISystem
{
public:
    UISystem(PostOffice& messenger, PlayerEntity& player)
    :
    p_post_office(&messenger), p_player(&player)
    {
        auto top_bar = std::make_shared<UIElement>();
        auto fuel_bars = std::make_shared<UIElement>();
        auto health_bars = std::make_shared<UIElement>();
        

        auto health_bar = std::make_shared<SpriteUIELement>();
        auto shield_bar = std::make_shared<SpriteUIELement>();
        
        auto fuel_bar = std::make_shared<SpriteUIELement>();
        auto boost_bar = std::make_shared<SpriteUIELement>();
        
        fuel_bars->addChildren(fuel_bar, boost_bar);
        health_bars->addChildren(health_bar, shield_bar);
        
        fuel_bars->layout = Layout::Y;
        fuel_bars->align = Alignement::Left;
        health_bars->layout = Layout::Y;
        health_bars->align = Alignement::Right;

        top_bar->bounding_box = {};
        top_bar->layout = Layout::X;
        top_bar->align = Alignement::Top;
        top_bar->addChildren(fuel_bars, health_bars);
        
        
        ui.root->addChildren(top_bar);
    }

    void draw(Renderer& window)
    {

    }

private:
    UIDocument ui;

    PlayerEntity* p_player;
    PostOffice* p_post_office;
};
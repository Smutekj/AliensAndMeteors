#pragma once

#include "Menu/UIDocument.h"
#include "PostOffice.h"

class UISystem
{
public:
    UISystem(Renderer &window, PostOffice &messenger, PlayerEntity *player)
        : ui(window), p_post_office(&messenger), p_player(player)
    {
        auto top_bar = std::make_shared<UIElement>();
        auto fuel_bars = std::make_shared<UIElement>();
        auto health_bars = std::make_shared<UIElement>();

        auto health_bar = std::make_shared<SpriteUIELement>();
        auto shield_bar = std::make_shared<SpriteUIELement>();
        health_bar->bounding_box = {0, 0, 200, 40};
        shield_bar->bounding_box = {0, 0, 200, 40};
        auto fuel_bar = std::make_shared<SpriteUIELement>();
        auto boost_bar = std::make_shared<SpriteUIELement>();
        fuel_bar->bounding_box = {0, 0, 200, 50};
        boost_bar->bounding_box = {0, 0, 200, 40};


        fuel_bars->layout = Layout::Y;
        fuel_bars->align = Alignement::Left;
        fuel_bars->sizing = Sizing::SCALE_TO_FIT;
        fuel_bars->margin.x = {50};
        fuel_bars->id = "FB";
        health_bars->layout = Layout::Y;
        health_bars->align = Alignement::Right;
        health_bars->sizing = Sizing::SCALE_TO_FIT;
        health_bars->margin.x = {50};
        health_bars->id = "HB";

        fuel_bars->addChildren(fuel_bar, boost_bar);
        health_bars->addChildren(health_bar, shield_bar);

        top_bar->bounding_box = {0,0, window.getTargetSize().x, 120};
        top_bar->layout = Layout::X;
        top_bar->align = Alignement::Top;
        top_bar->addChildren(fuel_bars, health_bars);

        ui.root->bounding_box = {0, 0, window.getTargetSize().x, window.getTargetSize().y};
        ui.root->addChildren(top_bar);
        ui.root->content_align_y = Alignement::Center;
    }

    void draw(Renderer &window)
    {
        auto old_view = window.m_view;
        window.m_view = window.getDefaultView();
        window.m_view.setSize(window.m_view.getSize().x, -window.m_view.getSize().y);
        ui.drawUI();

        window.m_view = old_view;
    }

    void update(float dt)
    {
    }

private:
    UIDocument ui;

    PlayerEntity *p_player;
    PostOffice *p_post_office;
};
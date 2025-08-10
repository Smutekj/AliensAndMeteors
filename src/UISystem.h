#pragma once

#include "Menu/UIDocument.h"
#include "PostOffice.h"
#include "PostBox.h"
#include "ComponentSystem.h"

class Font;

class UISystem
{
public:
    UISystem(Renderer &window, TextureHolder &textures,
             PostOffice &messenger, PlayerEntity *player,
             Font &font, GameSystems &systems);

    void draw(Renderer &window);

    void update(float dt);

private:
    void addBossBar();

private:
    UIDocument ui;

    GameSystems *p_systems;
    PlayerEntity *p_player;
    PostOffice *p_post_office;

    std::unique_ptr<PostBox<StartedBossFightEvent>> m_boss_postbox;

    TextureHolder& m_textures;
    Renderer &window_canvas;

    int boss_id = -1;
};
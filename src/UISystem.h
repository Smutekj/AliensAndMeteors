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
             Font &font, GameWorld &world);

    void draw(Renderer &window);

    void update(float dt);

private:
    void addBossBar();
    void removeBossBar();

private:
    UIDocument ui;

    GameWorld *p_world;
    PlayerEntity *p_player;
    PostOffice *p_post_office;

    std::vector<std::unique_ptr<PostBoxI>> m_post_boxes;

    TextureHolder& m_textures;
    Renderer &window_canvas;

    int boss_id = -1;
};
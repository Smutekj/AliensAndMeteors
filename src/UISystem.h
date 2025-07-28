#pragma once

#include "Menu/UIDocument.h"
#include "PostOffice.h"

class UISystem
{
public:
    UISystem(Renderer &window, TextureHolder& textures, PostOffice &messenger, PlayerEntity *player);

    void draw(Renderer &window);

    void update(float dt);

private:
    UIDocument ui;

    PlayerEntity *p_player;
    PostOffice *p_post_office;

    Renderer& window_canvas;
};
#pragma once

#include <Rect.h>
#include <Utils/Vector2.h>

class Animation
{

public:
    Animation(utils::Vector2i texture_size, int n_sprites_x, int n_sprites_y,
              float life_time, int n_repeats = 1,
              bool inverted = false);

    void setFrameTime(int new_frame_time);
    void setLifeTime(int new_life_time);
    void setTime(int new_time);
    void update(float dt);
    Rect<int> getCurrentTextureRect() const;

private:
    void animateSprite();

private:
    float m_time = 0;
    float m_frame_time = 5;
    float m_life_time;

    int m_tex_x;
    int m_tex_y;

    int m_sprites_x;
    int m_sprites_y;

    int m_repeats_count = -1;

    int m_dx = -1;
    int m_dy = -1;

    bool m_inverted = false;

    utils::Vector2i m_texture_rect_size;
};

#pragma once

#include <SFML/Graphics/Rect.hpp>

class Animation{

    int m_time = 0;
    int m_frame_time = 5;
    int m_life_time;

    int m_tex_x;
    int m_tex_y;

    int m_sprites_x;
    int m_sprites_y;

    int m_repeats_count = -1;

    int m_dx = -1;
    int m_dy = -1; 

    bool m_inverted = false;

    sf::Vector2i m_texture_rect_size;

public:
    Animation(sf::Vector2i texture_size, int n_sprites_x, int n_sprites_y,
                         int life_time, int n_repeats = 1,
                         bool inverted = false);

    void setFrameTime(int new_frame_time);
    void setLifeTime(int new_life_time);

    void setTime(int new_time);
    bool isDone() const;
    void update();
    sf::IntRect getCurrentTextureRect() const;

private:
    void animateSprite();

};

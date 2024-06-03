#include "Animation.h"

    Animation::Animation(sf::Vector2i texture_size, int sprites_x, int sprites_y,
                         float life_time, int repeats_count, bool inverted )
        : m_tex_x(sprites_x - 1), m_tex_y(sprites_y - 1),
          m_sprites_x(sprites_x), m_sprites_y(sprites_y),
          m_life_time(life_time), m_repeats_count(repeats_count), m_inverted(inverted)
    {
        if (m_inverted)
        {
            m_dx = 1;
            m_dy = 1;
            m_tex_x = 0;
            m_tex_y = 0;
        }
        m_texture_rect_size = { texture_size.x /m_sprites_x , texture_size.y/m_sprites_y };
        m_frame_time = m_life_time / (m_sprites_x*m_sprites_y);
    }

    void Animation::setFrameTime(int new_m_frame_time)
    {
        m_frame_time = new_m_frame_time;
    }

    void Animation::setLifeTime(int new_m_life_time)
    {
        m_life_time = new_m_life_time;
        if (m_repeats_count == 1)
        {
            m_frame_time = new_m_life_time / (m_sprites_x * m_sprites_y);
        }
    }

    void Animation::setTime(int new_time)
    {
        m_time = new_time;
        int frame_ind = (new_time / m_frame_time);
        m_tex_x = frame_ind % m_sprites_x;
        m_tex_y = frame_ind / m_sprites_x;
    }

    void Animation::update(float dt)
    {
        m_time += dt;
        if (m_time >= m_frame_time)
        {
            m_time = 0.f;
            animateSprite();
        }
    }

    sf::IntRect Animation::getCurrentTextureRect() const
    {
        return {{m_texture_rect_size.x * m_tex_x, m_texture_rect_size.y * m_tex_y},
                {m_texture_rect_size.x, m_texture_rect_size.y}};
    }

    void Animation::animateSprite()
    {
        m_tex_x += m_dx;
        if (m_tex_x >= m_sprites_x || m_tex_x < 0)
        {
            m_tex_y += m_dy;
            m_tex_x = (m_tex_x + m_sprites_x) % m_sprites_x;
        }
        if (m_tex_y >= m_sprites_y || m_tex_y < 0)
        {
            m_tex_y = (m_tex_y + m_sprites_y) % m_sprites_y;
        }

        if (m_inverted)
        {
            if (m_tex_x == 0 && m_tex_y == 0)
            {
                m_repeats_count--;
            }
        }
        else
        {
            if (m_tex_x == m_sprites_x - 1 && m_tex_y == m_sprites_y - 1)
            {
                m_repeats_count--;
            }
        }
    }


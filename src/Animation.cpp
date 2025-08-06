#include "Animation.h"

#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>

AnimationSystem::AnimationSystem(ContiguousColony<AnimationComponent, int> &comps, std::filesystem::path animations_dir)
    : m_components(comps)
{
    m_atlases.setBaseDirectory(animations_dir);
}

void AnimationSystem::registerAnimation(std::string atlas_id, AnimationId id, std::filesystem::path animation_datafile)
{
    if (!m_atlases.get(atlas_id))
    {
        m_atlases.add(atlas_id, atlas_id); //! yes the id and name should conicide
    }

    using json = nlohmann::json;
    try
    {
        std::ifstream file(animation_datafile);
        json animation_file = json::parse(file);

        m_frame_data[id] = {m_atlases.get(atlas_id), {}};

        auto& rects = m_frame_data[id].tex_rects;
        std::vector<int> frame_nums;
        int min_num = std::numeric_limits<int>::max();
        //! find frame numbers from filenames
        auto frames = animation_file.at("frames");
        for (auto &frame_js : frames.items())
        {
            nlohmann::json frame = frame_js.value();
            std::string filename = frame.at("filename").template get<std::string>();
            auto id_pos = filename.find_last_not_of("0123456789");
            
            if (id_pos == std::string::npos)
            {
                id_pos = 0;
            }else{
                id_pos++; //! the first number is one to the right
            }
            frame_nums.push_back(std::stoi(filename.substr(id_pos)));
            min_num = std::min(frame_nums.back(), min_num);
        }
        std::for_each(frame_nums.begin(), frame_nums.end(), [min_num](auto& num){num -= min_num;});
        
        rects.resize(frames.size());
        int i = 0;
        for (auto &frame_js : frames.items())
        {
            // int id = frame_js.key();
            nlohmann::json frame = frame_js.value();
            int x = frame["frame"].at("x").template get<int>();
            int y = frame["frame"].at("y").template get<int>();
            int w = frame["frame"].at("w").template get<int>();
            int h = frame["frame"].at("h").template get<int>();
            rects[frame_nums[i]] = {x, y, w, h};
            i++;
        }

    }
    catch (std::exception &e)
    {
        std::cout << "Error at animation registration: " << e.what() << std::endl;
    }
}

void AnimationSystem::update(float dt)
{
    for (auto &comp : m_components.data)
    {
        comp.time += dt;
        if (comp.time > comp.cycle_duration / m_frame_data.at(comp.id).tex_rects.size())
        {
            comp.time = 0.;
            comp.n_repeats_left--;
            comp.tex_rect = getNextFrame(comp);
        }
    }
}

Rect<int> AnimationSystem::getNextFrame(AnimationComponent &comp)
{
    auto &frames = m_frame_data.at(comp.id).tex_rects;
    auto frame_count = frames.size();
    comp.texture_size = m_frame_data.at(comp.id).p_texture->getSize();
    comp.texture_id = m_frame_data.at(comp.id).p_texture->getHandle();
    comp.current_frame_id = (comp.current_frame_id + 1) % frame_count;
    return frames.at(comp.current_frame_id);
}

Animation::Animation(utils::Vector2i texture_size, int sprites_x, int sprites_y,
                     float life_time, int repeats_count, bool inverted)
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
    m_texture_rect_size = {texture_size.x / m_sprites_x, texture_size.y / m_sprites_y};
    m_frame_time = m_life_time / (m_sprites_x * m_sprites_y);
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

Rect<int> Animation::getCurrentTextureRect() const
{
    return {m_texture_rect_size.x * m_tex_x, m_texture_rect_size.y * m_tex_y,
            m_texture_rect_size.x, m_texture_rect_size.y};
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

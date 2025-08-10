#pragma once

#include <Rect.h>
#include <Utils/Vector2.h>

#include <filesystem>

#include "Components.h"
#include "Systems/System.h"
#include "Texture.h"

class AnimationSystem : public SystemI
{
public:
    AnimationSystem(ContiguousColony<AnimationComponent, int> &comps,
                    std::filesystem::path animations_dir,
                    std::filesystem::path animations_data_dir);

    void registerAnimation(std::string atlas_id, AnimationId id, std::filesystem::path animation_datafile);

    virtual void preUpdate(float dt, EntityRegistryT &entities) override {}
    virtual void update(float dt) override;
    virtual void postUpdate(float dt, EntityRegistryT &entities) override {}

private:
    Rect<int> getNextFrame(AnimationComponent &comp);

private:
    struct FrameData
    {
        std::shared_ptr<Texture> p_texture;
        std::vector<Rect<int>> tex_rects;
    };

    ContiguousColony<AnimationComponent, int> &m_components;
    std::unordered_map<AnimationId, FrameData> m_frame_data;
    TextureHolder m_atlases;
    std::filesystem::path m_animations_data_dir;
};

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

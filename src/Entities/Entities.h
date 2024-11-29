#pragma once

#include <memory>
#include <deque>
#include <unordered_map>

#include <Sprite.h>

#include "../GameObject.h"

class BoidAI2;
class PlayerEntity;
class GameWorld;
class Animation;

namespace Collisions
{
    class CollisionSystem;
}

class GridNeighbourSearcher;

class Explosion : public GameObject
{

public:
    float m_explosion_radius = 5.f;
    float m_max_explosion_radius = 25.f;

    void setType(std::string texture_id);
    explicit Explosion(GameWorld *world, TextureHolder &textures);
    virtual ~Explosion() override;

    float getTimeLeftFraciton() const
    {
        return (m_life_time - m_time) / m_life_time;
    }

    virtual void update(float dt) override;
    virtual void onCreation() override;
    virtual void onDestruction() override;
    virtual void draw(LayersHolder &target) override;
    virtual void onCollisionWith(GameObject &obj, CollisionData &c_data) override;

public:
    bool m_is_expanding = true;

private:
    std::unique_ptr<Animation> m_animation;

    float m_min_dmg = 0.f;
    float m_max_dmg = 5.f;

    const float max_vel = 100.f;
    const float max_acc = 20.f;

    float m_time = 0.f;
    float m_life_time = 1.;
    Sprite m_explosion_sprite;
};

class EMP : public GameObject
{

public:
    float m_explosion_radius = 10.f;
    float m_max_explosion_radius = 20.f;

    EMP(GameWorld *world, TextureHolder &textures, Collisions::CollisionSystem *collider);
    virtual ~EMP() override;

    virtual void update(float dt) override;
    virtual void onCreation() override;
    virtual void onDestruction() override;
    virtual void draw(LayersHolder &target) override;
    virtual void onCollisionWith(GameObject &obj, CollisionData &c_data) override;

private:
    void onExplosion();

private:
    std::unique_ptr<Animation> m_animation;

    float m_min_dmg = 0.f;
    float m_max_dmg = 5.f;

    const float max_vel = 100.f;
    const float max_acc = 20.f;

    float m_time = 0.f;
    float m_life_time = 1.;

    Collisions::CollisionSystem *m_collider;

    bool m_is_ticking = true;

    Sprite m_texture_rect;
};

class ExplosionAnimation : public GameObject
{

public:
    float m_explosion_radius = 25.f;

    explicit ExplosionAnimation(GameWorld *world, TextureHolder &textures);
    virtual ~ExplosionAnimation() override;

    virtual void update(float dt) override;
    virtual void onCreation() override;
    virtual void onDestruction() override;
    virtual void draw(LayersHolder &target) override;
    virtual void onCollisionWith(GameObject &obj, CollisionData &c_data) override;

private:
    std::unique_ptr<Animation> m_animation;

    float m_min_dmg = 0.f;
    float m_max_dmg = 5.f;

    const float max_vel = 100.f;
    const float max_acc = 20.f;

    float m_life_time = 1.;
};

class Heart : public GameObject
{

public:
    Heart(GameWorld *world, TextureHolder &textures);
    virtual ~Heart() override;

    virtual void update(float dt) override;
    virtual void onCreation() override;
    virtual void onDestruction() override;
    virtual void draw(LayersHolder &target) override;
    virtual void onCollisionWith(GameObject &obj, CollisionData &c_data) override;

private:
    utils::Vector2f m_acc;
    const float max_vel = 100.f;
    const float max_acc = 20.f;
    float m_time = 0.f;
    float m_life_time = 10.;
};


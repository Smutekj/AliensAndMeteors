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

class Boss : public GameObject
{

    enum class State
    {

        Patroling,
        Shooting,
        ShootingLasers,
        ThrowingBombs,
    };

    State m_state = State::Patroling;

    utils::Vector2f m_acc;

    PlayerEntity *m_player;

    Collisions::CollisionSystem *m_collision_system;

    float m_shooting_cooldown = 3.f;
    float m_bombing_cooldown = 0.5f;
    float m_lasering_cooldown = 10.f;

    int m_bomb_count = 0;
    float m_shooting_timer = 0.f;

public:
    float max_vel = 30.f;
    const float max_acc = 100.f;
    float m_vision_radius = 150.f;

    float m_health = 50;
    utils::Vector2f m_impulse = {0, 0};
    utils::Vector2f m_target_pos;

    Boss(GameWorld *world, TextureHolder &textures, PlayerEntity *player);
    virtual ~Boss() override;

    virtual void update(float dt) override;
    virtual void onCreation() override;
    virtual void onDestruction() override;
    virtual void draw(LayersHolder &target) override;
    virtual void onCollisionWith(GameObject &obj, CollisionData &c_data) override;

private:
    void shootAtPlayer();
    void throwBombsAtPlayer();
    void shootLasers();
};

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

private:
    std::unique_ptr<Animation> m_animation;

    float m_min_dmg = 0.f;
    float m_max_dmg = 5.f;

    const float max_vel = 100.f;
    const float max_acc = 20.f;

    float m_time = 0.f;
    float m_life_time = 1.;
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

class Enemy;

class SpaceStation : public GameObject
{

public:
    SpaceStation(GameWorld *world, TextureHolder &textures);
    virtual ~SpaceStation() override;

    virtual void update(float dt) override;
    virtual void onCreation() override;
    virtual void onDestruction() override;
    virtual void draw(LayersHolder &target) override;
    virtual void onCollisionWith(GameObject &obj, CollisionData &c_data) override;

private:
    std::vector<Enemy *> m_produced_ships;
    float m_time = 0.f;
    float m_spawn_timer = 2.f;

    float m_max_health = 100.f;
    float m_health = m_max_health;
};

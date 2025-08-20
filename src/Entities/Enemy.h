#pragma once

#include <unordered_map>

#include <Sprite.h>

#include "../GameObject.h"
#include "../ComponentSystem.h"

class BoidAI2;
struct PlayerEntity;
class GameWorld;
class Animation;

namespace Collisions
{
    class CollisionSystem;
}

class Enemy : public GameObject
{

public:
    Enemy() = default;
    Enemy(GameWorld *world, TextureHolder &textures, PlayerEntity *player, GameSystems &systems);
    Enemy(const Enemy &e) = default;
    Enemy &operator=(Enemy &e) = default;
    Enemy &operator=(Enemy &&e) = default;

    virtual ~Enemy() override;

    virtual void update(float dt) override;
    virtual void onCreation() override;
    virtual void onDestruction() override;
    virtual void draw(LayersHolder &target) override;
    virtual void onCollisionWith(GameObject &obj, CollisionData &c_data) override;

public:
    bool m_deactivated = false;
    float m_deactivated_time = 1.f;

    float max_impulse_vel = 40.f;

    utils::Vector2f m_impulse = {0, 0};
    utils::Vector2f m_target_pos;

    Sprite m_sprite;

private:
    GameSystems *m_systems;

    PlayerEntity *m_player = nullptr;
};

class SpaceStation : public GameObject
{

public:
    SpaceStation() = default;
    SpaceStation(GameWorld *world, TextureHolder &textures, PlayerEntity *player, GameSystems &systems);
    SpaceStation(const SpaceStation &e) = default;
    SpaceStation &operator=(SpaceStation &e) = default;
    SpaceStation &operator=(SpaceStation &&e) = default;
    virtual ~SpaceStation() override;

    virtual void update(float dt) override;
    virtual void onCreation() override;
    virtual void onDestruction() override;
    virtual void draw(LayersHolder &target) override;
    virtual void onCollisionWith(GameObject &obj, CollisionData &c_data) override;

private:
    std::vector<int> m_produced_ships;

    GameSystems *p_systems;
    float m_spawn_timer = 2.f;

    float m_max_health = 100.f;
    float m_health = m_max_health;
};

class Boss : public GameObject
{

    enum class MotionState
    {
        Pursuing,
        Searching,
    };

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
    float m_lasering_cooldown = 3.f;

    int m_bomb_count = 0;
    float m_shooting_timer = 0.f;
    float m_shooting_timer2 = 0.f;

    bool m_is_recharging = false;
    float m_recharge_time = 7.;

public:
    float m_orig_max_vel = 90.f;
    float m_vision_radius = 70.f;

    float m_health = 50;
    float m_max_health = 50;
    utils::Vector2f m_impulse = {0, 0};
    utils::Vector2f m_target_pos;

    Boss() = default;
    Boss(GameWorld *world, TextureHolder &textures, PlayerEntity *player = nullptr);
    Boss(const Boss &e) = default;
    Boss &operator=(Boss &e) = default;
    Boss &operator=(Boss &&e) = default;
    virtual ~Boss() override;

    virtual void update(float dt) override;
    virtual void onCreation() override;
    virtual void onDestruction() override;
    virtual void draw(LayersHolder &target) override;
    virtual void onCollisionWith(GameObject &obj, CollisionData &c_data) override;

private:
    void aiWhenRecharged(float dt);
    void shootAtPlayer();
    void throwBombsAtPlayer();
    void shootLasers();
    void shootLaserAtPlayer();
};

class ProjectileFactory;

class Turret : public GameObject
{

public:
    Turret() = default;
    Turret(GameWorld *world, TextureHolder &textures, PlayerEntity *player = nullptr);
    Turret(const Turret &e) = default;
    Turret &operator=(Turret &e) = default;
    Turret &operator=(Turret &&e) = default;
    virtual ~Turret() override;

    virtual void update(float dt) override;
    virtual void onCreation() override;
    virtual void onDestruction() override;
    virtual void draw(LayersHolder &target) override;
    virtual void onCollisionWith(GameObject &obj, CollisionData &c_data) override;

private:
    float m_time;

    std::unique_ptr<ProjectileFactory> m_factory;

    PlayerEntity *p_player = nullptr;

    float m_shooting_range = 169.;

    float m_turn_speed = 49; //! degrees per second;

    float m_spawn_timer = 2.f;

    float m_max_health = 100.f;
    float m_health = m_max_health;
};

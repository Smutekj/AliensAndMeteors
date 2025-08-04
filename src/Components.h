
#pragma once

#include "Vector2.h"
#include "Systems/TimedEvent.h"

class GameObject;

struct BoidComponent
{
    utils::Vector2f target_pos;
    utils::Vector2f pos;
    utils::Vector2f vel = {0,0};
    utils::Vector2f acc = {0,0}; //! resulting acceleration;
    float boid_radius = 40.f;
};

struct TargetComponent
{
    GameObject* p_target = nullptr;
    utils::Vector2f target_pos;
    float targetting_strength = 1.f;
};

struct AvoidMeteorsComponent
{
    utils::Vector2f target_pos;
    utils::Vector2f pos;
    utils::Vector2f vel = {0,0};
    utils::Vector2f acc = {0,0}; //! resulting acceleration;
    float radius = 50.f;
};

struct HealthComponent
{
    float hp;
    float max_hp;
    float hp_regen;
};

struct ShieldComponent
{
    float shield;
    float max_shield;
    float shield_regen;
};

struct GunComponent
{
    float cooldown;
    float damage;
    bool homing = false;
};

struct BombThrowingComponent
{
    float cooldown;
    float explosion_delay;
    float damage;
};

struct LaserGunComponent
{
    float cooldown;
    float damage;
    float max_length;
    float max_width;
    float slowing_factor;
};

struct AIComponent
{
    
};

struct TimedEventComponent
{
    int addEvent(TimedEvent event)
    {
        next_id++;
        events.insert({next_id, event});
        return next_id;
    }
    std::unordered_map<int, TimedEvent> events;
    int next_id = 0; //! TODO: this is fucked up and will fix it later 
};

enum class ComponentId
{
    Boid,
    Shield,
    Hp
};
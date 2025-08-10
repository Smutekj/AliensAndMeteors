
#pragma once

#include "Vector2.h"
#include "Systems/TimedEvent.h"
#include "Rect.h"

class GameObject;

enum class ObjectType
{
    Enemy,
    Bullet,
    Missile,
    Bomb,
    Laser,
    Meteor,
    Heart,
    SpaceStation,
    Explosion,
    Player,
    Flag,
    Boss,
    Trigger,
    EMP,
    Count
};



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

#include "Polygon.h"


struct CollisionShape
{
    std::vector<Polygon> convex_shapes;

    AABB getBoundingRect() const
    {
        assert(convex_shapes.size() > 0);
        AABB box = convex_shapes.at(0).getBoundingRect();
        for (std::size_t i = 1; i < convex_shapes.size(); ++i)
        {
            box = makeUnion(box, convex_shapes.at(i).getBoundingRect());
        }
        return box;
    }
};

struct CollisionComponent
{
    CollisionShape shape;
    ObjectType type;
    std::function<void(int, ObjectType)> on_collision = [](auto, auto){};
};

enum class AnimationId
{
    Shield,
    BlueExplosion,
    PurpleExplosion,
    GreenBeam,
};

struct AnimationComponent
{
    AnimationId id;
    unsigned int texture_id = 0;
    float time = 0.;
    float cycle_duration = 1.;
    int current_frame_id = 0;
    int n_repeats_left = 1;
    Rect<int> tex_rect = {0,0,0,0};
    utils::Vector2i texture_size = {1,1};
};

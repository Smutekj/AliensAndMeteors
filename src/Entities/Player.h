#pragma once

// #include "../Geometry.h"
#include "../GameObject.h"

#include <Renderer.h>
#include <Particles.h>

class GameWorld;

struct PlayerEntity : public GameObject
{
public:
    PlayerEntity() = default;
    PlayerEntity(GameWorld *world, TextureHolder &textures, Collisions::CollisionSystem *collider = nullptr, PlayerEntity *player = nullptr);
    PlayerEntity(const PlayerEntity &e) = default;
    PlayerEntity &operator=(PlayerEntity &e) = default;
    PlayerEntity &operator=(PlayerEntity &&e) = default;

    virtual ~PlayerEntity() = default;

    virtual void update(float dt) override;
    virtual void draw(LayersHolder &target) override;
    virtual void onCreation() override;
    virtual void onDestruction() override;
    virtual void onCollisionWith(GameObject &obj, CollisionData &c_data) override;

private:
    void fixAngle();
    void boost();

public:
    float speed = 0.f;
    float boost_max_speed = 80.f;
    float max_speed = 50.f;
    bool is_boosting = false;

    float m_laser_timer = 0.f;

    bool m_is_shooting_laser = false;
    bool m_is_turning_left = false;
    bool m_is_turning_right = false;

    float boost_factor = 2.6f;
    float slowing_factor = 0.03f;
    float acceleration = 1.5f;
    float m_angle_vel = 270.69f;

    float boost_time = 0.f;
    float max_boost_time = 100.f;

    float boost_heat = 0.f;
    float max_boost_heat = 100.f;

    float m_radius = 3.f;

    float health = 100;
    float max_health = 100;

    float m_deactivated_time = -1.f;

    Sprite m_player_shape;
    std::shared_ptr<Particles> m_particles_left;
    std::shared_ptr<Particles> m_particles_right;
};

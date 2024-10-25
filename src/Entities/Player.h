#pragma once


// #include "../Geometry.h"
#include "../GameObject.h"

#include <Renderer.h>
#include <Particles.h>


class GameWorld;

struct PlayerEntity : public GameObject
{
    public:
    PlayerEntity(GameWorld *world, TextureHolder &textures);
    virtual ~PlayerEntity() = default;

    virtual void update(float dt) override;
    virtual void draw(LayersHolder &target) override;
    virtual void onCreation() override;
    virtual void onDestruction() override;
    virtual void onCollisionWith(GameObject& obj, CollisionData& c_data) override;

private:
    void fixAngle();
    void boost();

public:
    float speed = 0.f;
    float max_speed = 5.f;
    bool is_boosting = false;
    float boost_factor = 2.f;
    float slowing_factor = 0.03f;
    float acceleration = 1.5f;

    float boost_time = 0.f;
    float max_boost_time = 100.f;

    float boost_heat = 0.f;
    float max_boost_heat = 100.f;

    float m_radius = 3.f;

    float health = 100;
    float max_health = 100;

    float m_deactivated_time = -1.f;

    Sprite m_player_shape;
    std::unique_ptr<Particles> m_particles_left;
    std::unique_ptr<Particles> m_particles_right;

};

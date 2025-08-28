#pragma once

// #include "../Geometry.h"
#include "../GameObject.h"

#include <Renderer.h>
#include <Particles.h>


enum class BoosterState
    {
        Boosting,
        Ready,
        CoolingDown,
        Disabled,
    };


class GameWorld;

struct PlayerEntity : public GameObject
{
public:
    PlayerEntity() = default;
    PlayerEntity(GameWorld *world, TextureHolder &textures, PlayerEntity *player = nullptr);
    PlayerEntity(const PlayerEntity &e) = default;
    PlayerEntity &operator=(PlayerEntity &e) = default;
    PlayerEntity &operator=(PlayerEntity &&e) = default;

    virtual ~PlayerEntity() = default;

    virtual void update(float dt) override;
    virtual void draw(LayersHolder &target) override;
    virtual void onCreation() override;
    virtual void onDestruction() override;
    virtual void onCollisionWith(GameObject &obj, CollisionData &c_data) override;

    float getHp() const;
    float getHpRatio() const;

    void activateShield();
    void deactivateShield();

    void onBoostDown();
    void onBoostUp();

private:
    void fixAngle();
    void boost(float dt);

    
public:
    float speed = 0.f;
    float boost_max_speed = 150.f;
    float max_speed = 100.f;
    BoosterState booster = BoosterState::Ready;

    int m_money = 100;

    float m_laser_timer = 0.f;

    bool m_is_shooting_laser = false;
    bool m_is_turning_left = false;
    bool m_is_turning_right = false;

    float boost_factor = 2.6f;
    float slowing_factor = 0.3f;
    float acceleration = 1.5f;
    float m_angle_vel = 270.69f;

    float m_max_fuel = 100.;
    float m_fuel = 100.;

    float boost_time = 0.f;
    float max_boost_time = 100.f;

    float boost_heat = 0.f;
    float max_boost_heat = 100.f;

    float health = 100;
    float max_health = 100;

    float shield_lifetime = 5.;
    float max_shield_hp = 10;
    float shield_timeleft = shield_lifetime;
    bool shield_active = false;

    float m_deactivated_time = -1.f;

    Sprite m_player_shape;
    std::shared_ptr<Particles> m_particles_left;
    std::shared_ptr<Particles> m_particles_right;

    private:

    int m_shield_id = -1;
    Polygon meteor_detector;
};

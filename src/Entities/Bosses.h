#pragma once

#include "../GameObject.h"
#include "Player.h"

class Boss1 : public GameObject
{
    enum class State
    {
        EnteringFight,
        ShootingLasers,
        ShootingGuns,
        Exposed,
        ShootingBigLaser,
    };
public:
    Boss1() = default;
    Boss1(GameWorld *world, TextureHolder &textures, Collisions::CollisionSystem *collider, PlayerEntity *player);
    Boss1(const Boss1 &e) = default;
    Boss1 &operator=(Boss1 &e) = default;
    Boss1 &operator=(Boss1 &&e) = default;
    virtual ~Boss1() override = default;

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

    void changeState(State target_state);

private:
    std::unordered_map<State, std::function<void()>> m_on_state_change;

    PlayerEntity *p_player;

    State m_state = State::ShootingLasers;

    PlayerEntity *m_player;
    utils::Vector2f m_acc;

    Collisions::CollisionSystem *m_collision_system;

    float m_motion_time = 0.f;
    float m_motion_period = 6.f;  

    float m_bombing_cooldown = 0.5f;
    float m_lasering_cooldown = 3.f;

    int m_bomb_count = 0;
    float m_shooting_timer = 0.f;
    float m_shooting_timer2 = 0.f;

    bool m_is_recharging = false;
    float m_recharge_time = 7.;

public:
    float m_orig_max_vel = 90.f;
    float m_max_vel = 90.f;
    float max_acc = 130.f;
    float m_vision_radius = 70.f;

    float m_health = 50;
    float m_max_health = 50;
    utils::Vector2f m_impulse = {0, 0};
    utils::Vector2f m_target_pos;

};

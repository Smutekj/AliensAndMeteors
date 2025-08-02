// #pragma once

// #include "../GameObject.h"
// #include "Player.h"

// class Boss1 : public GameObject
// {

//     enum class State
//     {
//         EnteringFight,
//         ShootingLasers,
//         ShootingGuns,
//         Exposed,
//         ShootingBigLaser,
//     };


//     State m_state = State::EnteringFight;

    
//     PlayerEntity *m_player;
//     utils::Vector2f m_acc;
    
//     Collisions::CollisionSystem *m_collision_system;
    
//     float m_bombing_cooldown = 0.5f;
//     float m_lasering_cooldown = 3.f;
    
//     int m_bomb_count = 0;
//     float m_shooting_timer = 0.f;
//     float m_shooting_timer2 = 0.f;
    
//     bool m_is_recharging = false;
//     float m_recharge_time = 7.;
    
//     public:
//     float m_orig_max_vel = 90.f;
//     float m_max_vel = 90.f;
//     float max_acc = 130.f;
//     float m_vision_radius = 70.f;
    
//     float m_health = 50;
//     float m_max_health = 50;
//     utils::Vector2f m_impulse = {0, 0};
//     utils::Vector2f m_target_pos;

//     Boss1() = default;
//     Boss1(GameWorld *world, TextureHolder &textures, Collisions::CollisionSystem *collider = nullptr, PlayerEntity *player = nullptr);
//     Boss1(const Boss &e) = default;
//     Boss1 &operator=(Boss1 &e) = default;
//     Boss1 &operator=(Boss1 &&e) = default;
//     virtual ~Boss1() override;

//     virtual void update(float dt) override;
//     virtual void onCreation() override;
//     virtual void onDestruction() override;
//     virtual void draw(LayersHolder &target) override;
//     virtual void onCollisionWith(GameObject &obj, CollisionData &c_data) override;

//     PlayerEntity* p_player

// private:
//     void aiWhenRecharged(float dt);
//     void shootAtPlayer();
//     void throwBombsAtPlayer();
//     void shootLasers();
//     void shootLaserAtPlayer();
// };

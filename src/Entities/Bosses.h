#pragma once

#include "../GameObject.h"
#include "Player.h"
#include "../Animation.h"
#include "Factories.h"

class Boss1 : public GameObject
{
    enum class State
    {
        EnteringFight,
        ShootingLasers,
        ShootingGuns,
        Exposed,
        ShootingBigLaser,
        SpawningShips,
    };

public:
    Boss1() = default;
    Boss1(GameWorld *world, TextureHolder &textures, PlayerEntity *player);
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
    void activateShield();
    void deActivateShield();

    void changeState(State target_state);

private:
    int shield_id = -1;
    std::unordered_map<State, std::function<void()>> m_on_state_change;

    PlayerEntity *p_player;

    State m_state = State::ShootingLasers;

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

    ProjectileFactory m_projectile_factory;
    LaserFactory m_laser_factory;
    EnemyFactory m_enemy_factory;

public:
    float m_orig_max_vel = 90.f;
    float m_vision_radius = 70.f;

    float m_health = 50;
    float m_max_health = 50;
    utils::Vector2f m_impulse = {0, 0};
    utils::Vector2f m_target_pos;
};


template <class StateId>  
class StateMachine
{

    void registerTransition(StateId target_state, std::function<void()> callback)
    {
        m_on_state_change[target_state] = callback;
    }

    void changeState(StateId target_state)
    {
        if(m_on_state_change.contains(target_state))
        {
            m_on_state_change.at(target_state)();
        }
        
        m_history.push_back(m_current_state);
        if(m_history.size() > m_max_history_size)
        {
            m_history.pop_front();
        }

        m_current_state = target_state;
    }

    void undoStateChange()
    {
        if(!m_history.empty())
        {
            changeState(m_history.front());
            m_history.pop_front();
        }
    }


    
    std::unordered_map<StateId, std::function<void()>> m_on_state_change;
    std::unordered_map<StateId, std::unordered_set<StateId>> m_transitions;
    std::deque<StateId> m_history;
    StateId m_current_state;
    int m_max_history_size = 5;
};

class Weapon : public GameObject
{
    enum class AState
    {
        Shooting,
        Reloading,
        Ready,
        Moving,
    };

public:
    Weapon() = default;
    Weapon(GameWorld *world, TextureHolder &textures, PlayerEntity *player);
    Weapon(const Weapon &e) = default;
    Weapon &operator=(Weapon &e) = default;
    Weapon &operator=(Weapon &&e) = default;
    virtual ~Weapon() override = default;


    virtual void update(float dt) override;
    virtual void onCreation() override;
    virtual void onDestruction() override{}
    virtual void draw(LayersHolder &target) override;
    virtual void onCollisionWith(GameObject &obj, CollisionData &c_data) override;

    private:

    
    StateMachine<AState> m_state_machine;

};



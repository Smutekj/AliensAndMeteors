#pragma once

#include "core.h"

#include <memory>
#include <deque>
#include <unordered_map>

#include "GameObject.h"
#include "Polygon.h"

#include "Animation.h"

class BoidAI2;
class PlayerEntity;
class GameWorld;

namespace Collisions
{
    class CollisionSystem;
}

class GridNeighbourSearcher;

class Enemy : public GameObject
{

    float m_boid_radius = 30.f;

    sf::Vector2f m_acc;

    std::unique_ptr<BoidAI2> m_behaviour;

    PlayerEntity *m_player;

    GridNeighbourSearcher *m_neighbour_searcher;
    Collisions::CollisionSystem *m_collision_system;

    bool m_is_avoiding = false;

public:
    static std::unordered_map<Multiplier, float> m_force_multipliers;
    static std::unordered_map<Multiplier, float> m_force_ranges;

    float max_vel = 30.f;
    const float max_acc = 100.f;
    const float max_impulse_vel = 40.f;

    float m_health = 5;
    sf::Vector2f m_impulse = {0, 0};
    sf::Vector2f m_target_pos;
    std::vector<sf::Vector2f> m_cm;

    Enemy(GameWorld *world, TextureHolder &textures,
          Collisions::CollisionSystem &collider, GridNeighbourSearcher &m_ns, PlayerEntity *player);
    virtual ~Enemy() override;

    virtual void update(float dt) override;
    virtual void onCreation() override;
    virtual void onDestruction() override;
    virtual void draw(sf::RenderTarget &target) override;
    virtual void onCollisionWith(GameObject &obj, CollisionData &c_data) override;

    void setBehaviour();

private:
    void avoidMeteors();
    void boidSteering();
};


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

    sf::Vector2f m_acc;

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
    sf::Vector2f m_impulse = {0, 0};
    sf::Vector2f m_target_pos;

    Boss(GameWorld *world, TextureHolder &textures, PlayerEntity *player);
    virtual ~Boss() override;

    virtual void update(float dt) override;
    virtual void onCreation() override;
    virtual void onDestruction() override;
    virtual void draw(sf::RenderTarget &target) override;
    virtual void onCollisionWith(GameObject &obj, CollisionData &c_data) override;


private:

    void shootAtPlayer();
    void throwBombsAtPlayer();
    void shootLasers();

};



class Bullet2 : public GameObject
{

    sf::Vector2f m_acc;
    const float max_vel = 500.f;
    const float max_acc = 70.f;

    PlayerEntity *m_player = nullptr;

    float m_time = 0.f;
    float m_life_time = 10.;

    std::deque<sf::Vector2f> m_past_positions;

public:
    Bullet2(GameWorld *world, TextureHolder &textures, PlayerEntity *player = nullptr);
    virtual ~Bullet2() override;

    virtual void update(float dt) override;
    virtual void onCreation() override;
    virtual void onDestruction() override;
    virtual void draw(sf::RenderTarget &target) override;
    virtual void onCollisionWith(GameObject &obj, CollisionData &c_data) override;

    float getTime()const
    {
        return m_time;
    }
};

class Bomb2 : public GameObject
{

    std::unique_ptr<Animation> m_animation;

    float m_explosion_radius = 50.f;
    float m_min_dmg = 0.f;
    float m_max_dmg = 5.f;

    const float max_vel = 1000.f;
    const float max_acc = 20.f;

    Collisions::CollisionSystem *m_neighbour_searcher;

public:
    float m_life_time = 1.;
    
    Bomb2(GameWorld *world, TextureHolder &textures, Collisions::CollisionSystem &collider);
    virtual ~Bomb2() override;

    virtual void update(float dt) override;
    virtual void onCreation() override;
    virtual void onDestruction() override;
    virtual void draw(sf::RenderTarget &target) override;
    virtual void onCollisionWith(GameObject &obj, CollisionData &c_data) override;
};

class Laser2 : public GameObject
{

    float m_min_dmg = 0.f;
    float m_max_dmg = 5.f;
    float m_length = 0.f;
    float m_width = 3.f;

    const float max_vel = 100.f;
    const float max_acc = 20.f;

    float m_life_time = 1.;
    Collisions::CollisionSystem *m_neighbour_searcher;

    GameObject *m_owner = nullptr;

public:
    Laser2(GameWorld *world, TextureHolder &textures, Collisions::CollisionSystem &collider);
    virtual ~Laser2() override;

    virtual void update(float dt) override;
    virtual void onCreation() override;
    virtual void onDestruction() override;
    virtual void draw(sf::RenderTarget &target) override;
    virtual void onCollisionWith(GameObject &obj, CollisionData &c_data) override;

    void setOwner(GameObject *owner)
    {
        m_owner = owner;
    }
};

class Meteor : public GameObject
{

    const float max_vel = 50.f;
    const float max_impulse_vel = 40.f;

public:
    explicit Meteor(GameWorld *world, TextureHolder &textures);
    virtual ~Meteor() override;

    virtual void update(float dt) override;
    virtual void onCreation() override;
    virtual void onDestruction() override;
    virtual void draw(sf::RenderTarget &target) override;
    virtual void onCollisionWith(GameObject &obj, CollisionData &c_data) override;

private:
    void initializeRandomMeteor();

    Polygon generateRandomConvexPolygon(int n) const;
};

class Explosion : public GameObject
{

    std::unique_ptr<Animation> m_animation;

    float m_min_dmg = 0.f;
    float m_max_dmg = 5.f;

    const float max_vel = 100.f;
    const float max_acc = 20.f;

    float m_time = 0.f;
    float m_life_time = 1.;

public:
    float m_explosion_radius = 25.f;

    void setType(Textures::ID type)
    {
        if (type == Textures::ID::Explosion2)
        {
            auto texture_size = static_cast<sf::Vector2i>(m_textures.get(Textures::ID::Explosion2).getSize());
            m_animation = std::make_unique<Animation>(texture_size,
                                                      12, 1, m_life_time / 0.016f, 1, true);
        }
        else if (type == Textures::ID::Explosion)
        {
            auto texture_size = static_cast<sf::Vector2i>(m_textures.get(Textures::ID::Explosion2).getSize());
            m_animation = std::make_unique<Animation>(texture_size,
                                                      4, 4, m_life_time / 0.016f, 1, false);
        }
    }

    explicit Explosion(GameWorld *world, TextureHolder &textures);
    virtual ~Explosion() override;

    float getTimeLeftFraciton()const
    {
        return (m_life_time - m_time)/m_life_time; 
    }

    virtual void update(float dt) override;
    virtual void onCreation() override;
    virtual void onDestruction() override;
    virtual void draw(sf::RenderTarget &target) override;
    virtual void onCollisionWith(GameObject &obj, CollisionData &c_data) override;

private:
};

class ExplosionAnimation : public GameObject
{

    std::unique_ptr<Animation> m_animation;

    float m_min_dmg = 0.f;
    float m_max_dmg = 5.f;

    const float max_vel = 100.f;
    const float max_acc = 20.f;

    float m_life_time = 1.;

public:
    float m_explosion_radius = 25.f;

    explicit ExplosionAnimation(GameWorld *world, TextureHolder &textures);
    virtual ~ExplosionAnimation() override;

    virtual void update(float dt) override;
    virtual void onCreation() override;
    virtual void onDestruction() override;
    virtual void draw(sf::RenderTarget &target) override;
    virtual void onCollisionWith(GameObject &obj, CollisionData &c_data) override;

private:
};

class Heart : public GameObject
{

    sf::Vector2f m_acc;
    const float max_vel = 100.f;
    const float max_acc = 20.f;

    float m_time = 0.f;
    float m_life_time = 10.;

public:
    Heart(GameWorld *world, TextureHolder &textures);
    virtual ~Heart() override;

    virtual void update(float dt) override;
    virtual void onCreation() override;
    virtual void onDestruction() override;
    virtual void draw(sf::RenderTarget &target) override;
    virtual void onCollisionWith(GameObject &obj, CollisionData &c_data) override;
};

class SpaceStation : public GameObject
{

    float m_time = 0.f;
    float m_spawn_timer = 2.f;

    const float m_max_health = 100.f;
    float m_health = m_max_health;

public:
    SpaceStation(GameWorld *world, TextureHolder &textures);
    virtual ~SpaceStation() override;

    virtual void update(float dt) override;
    virtual void onCreation() override;
    virtual void onDestruction() override;
    virtual void draw(sf::RenderTarget &target) override;
    virtual void onCollisionWith(GameObject &obj, CollisionData &c_data) override;
};

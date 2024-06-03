#pragma once

#include <memory>
#include <deque>
#include <unordered_map>

#include <SFML/Graphics/RectangleShape.hpp>

#include "../GameObject.h"


class PlayerEntity;
class GameWorld;
class Animation;

namespace Collisions
{
    class CollisionSystem;
}


class Bullet : public GameObject
{
public:
    Bullet(GameWorld *world, TextureHolder &textures, PlayerEntity *player = nullptr);
    virtual ~Bullet() override;

    virtual void update(float dt) override;
    virtual void onCreation() override;
    virtual void onDestruction() override;
    virtual void draw(sf::RenderTarget &target) override;
    virtual void onCollisionWith(GameObject &obj, CollisionData &c_data) override;

    float getTime() const;

private:
    sf::Vector2f m_acc;
    const float max_vel = 500.f;
    const float max_acc = 70.f;

    PlayerEntity *m_player = nullptr;

    float m_time = 0.f;
    float m_life_time = 10.;

    std::deque<sf::Vector2f> m_past_positions;
};

class Bomb : public GameObject
{

public:
    float m_life_time = 1.;

    Bomb(GameWorld *world, TextureHolder &textures, Collisions::CollisionSystem &collider);
    virtual ~Bomb() override;

    virtual void update(float dt) override;
    virtual void onCreation() override;
    virtual void onDestruction() override;
    virtual void draw(sf::RenderTarget &target) override;
    virtual void onCollisionWith(GameObject &obj, CollisionData &c_data) override;

private:
    std::unique_ptr<Animation> m_animation;

    float m_explosion_radius = 25.f;
    float m_min_dmg = 0.f;
    float m_max_dmg = 5.f;

    const float max_vel = 1000.f;
    const float max_acc = 20.f;

    Collisions::CollisionSystem *m_neighbour_searcher;
};

class Laser : public GameObject
{

public:
    Laser(GameWorld *world, TextureHolder &textures, Collisions::CollisionSystem &collider);
    virtual ~Laser() override;

    virtual void update(float dt) override;
    virtual void onCreation() override;
    virtual void onDestruction() override;
    virtual void draw(sf::RenderTarget &target) override;
    virtual void onCollisionWith(GameObject &obj, CollisionData &c_data) override;

    void setOwner(GameObject *owner)
    {
        m_owner = owner;
    }
    const GameObject *getOwner() const
    {
        return m_owner;
    }

private:
    float m_min_dmg = 0.f;
    float m_max_dmg = 5.f;
    float m_length = 0.f;
    float m_width = 3.f;

    const float max_vel = 100.f;
    const float max_acc = 20.f;

    float m_life_time = 1.;
    Collisions::CollisionSystem *m_neighbour_searcher;

    GameObject *m_owner = nullptr;
};
#pragma once

#include <memory>
#include <deque>
#include <unordered_map>

#include "../GameObject.h"

class PlayerEntity;
class GameWorld;
class Animation;

namespace Collisions
{
    class CollisionSystem;
}

enum class BulletType
{
    Lightning,
    Fire,
    Frost,
    Laser,
};

class Bullet : public GameObject
{
public:
    Bullet(GameWorld *world, TextureHolder &textures, PlayerEntity *player = nullptr);
    virtual ~Bullet() override;

    virtual void update(float dt) override;
    virtual void onCreation() override;
    virtual void onDestruction() override;
    virtual void draw(LayersHolder &target) override;
    virtual void onCollisionWith(GameObject &obj, CollisionData &c_data) override;

    float getTime() const;

    void setTarget(GameObject *new_target);
    GameObject *getTarget() const;
    void setBulletType(BulletType type);

    
    float m_max_vel = 100.f;
    float m_max_acc = 30.f;
private:
    static std::unordered_map<BulletType, std::string> m_type2shader_id;

private:
    utils::Vector2f m_acc;

    BulletType m_type = BulletType::Lightning;

    GameObject *m_target = nullptr;

    float m_tail_timer = 0.f;
    float m_time = 0.f;
    float m_life_time = 10.;

    std::deque<utils::Vector2f> m_past_positions;
};


class Bomb : public GameObject
{

public:
    Bomb(GameWorld *world, TextureHolder &textures, Collisions::CollisionSystem &collider);
    virtual ~Bomb() override;

    virtual void update(float dt) override;
    virtual void onCreation() override;
    virtual void onDestruction() override;
    virtual void draw(LayersHolder &target) override;
    virtual void onCollisionWith(GameObject &obj, CollisionData &c_data) override;

public:
    float m_life_time = 3.;
    float m_acc = 10.;
private:
    std::unique_ptr<Animation> m_animation;

    float m_explosion_radius = 5.f;
    float m_min_dmg = 0.f;
    float m_max_dmg = 5.f;

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
    virtual void draw(LayersHolder &target) override;
    virtual void onCollisionWith(GameObject &obj, CollisionData &c_data) override;

    void setOwner(GameObject *owner)
    {
        m_owner = owner;
    }
    const GameObject *getOwner() const
    {
        return m_owner;
    }

    float m_min_dmg = 0.f;
    float m_max_dmg = 1.f;
    float m_max_length = 100.f;
    float m_width = 3.f;
    float m_life_time = 1.;

    bool m_rotates_with_owner = true;
private:
    float m_length = 0.f;
    float m_time = 0.;

    Collisions::CollisionSystem *m_neighbour_searcher;

    GameObject *m_owner = nullptr;
};
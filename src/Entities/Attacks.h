#pragma once

#include <memory>
#include <deque>
#include <unordered_map>

#include "../GameObject.h"

struct PlayerEntity;
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
    Bullet() = default;
    Bullet(GameWorld *world, TextureHolder &textures , PlayerEntity *player = nullptr);
    Bullet(const Bullet &e) = default;
    Bullet &operator=(Bullet &e) = default;
    Bullet &operator=(Bullet &&e) = default;
    virtual ~Bullet() override = default;

    virtual void update(float dt) override;
    virtual void onCreation() override;
    virtual void onDestruction() override;
    virtual void draw(LayersHolder &target) override;
    virtual void onCollisionWith(GameObject &obj, CollisionData &c_data) override;

    float getTime() const;

    void setTarget(GameObject *new_target);
    GameObject *getTarget() const;
    void setBulletType(BulletType type);

public:
    Sprite m_sprite;
private:
    static std::unordered_map<BulletType, std::string> m_type2shader_id;

private:

    BulletType m_type = BulletType::Lightning;

    GameObject *m_target = nullptr;
    GameObject *m_shooter = nullptr;

    float m_time = 0.f;

};

class Bomb : public GameObject
{

public:
    Bomb() = default;
    Bomb(GameWorld *world, TextureHolder &textures, PlayerEntity *player = nullptr);
    Bomb(const Bomb &e) = default;
    Bomb &operator=(Bomb &e) = default;
    Bomb &operator=(Bomb &&e) = default;

    virtual ~Bomb() override = default;

    virtual void update(float dt) override;
    virtual void onCreation() override;
    virtual void onDestruction() override;
    virtual void draw(LayersHolder &target) override;
    virtual void onCollisionWith(GameObject &obj, CollisionData &c_data) override;

public:
    float m_life_time = 3.;
    float m_acc = 10.;

private:
    std::shared_ptr<Animation> m_animation;

    float m_explosion_radius = 5.f;
    float m_min_dmg = 0.f;
    float m_max_dmg = 5.f;

    Collisions::CollisionSystem *m_neighbour_searcher;
};

class Laser : public GameObject
{

public:
    Laser() = default;

    Laser(GameWorld *world, TextureHolder &textures, PlayerEntity *player = nullptr);
    Laser(const Laser &e) = default;
    Laser(Laser &&other) = default;

    Laser &operator=(Laser &e) = default;
    Laser &operator=(Laser &&e) = default;
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

private:
    void stopAgainst(ObjectType type);
public:

    float m_min_dmg = 0.f;
    float m_max_dmg = 1.f;
    float m_max_length = 100.f;
    float m_max_width = 3.f;
    float m_life_time = 1.;

    bool m_rotates_with_owner = true;
    
    utils::Vector2f m_offset = {0,0};

    std::function<void(float )> m_updater = [](float ){};    

    float m_length = 0.f;
    float m_width = 0.f;

    std::vector<ObjectType> m_stopping_types = {ObjectType::Meteor};

    ColorByte m_laser_color = {255, 255, 255, 255};
private:
    float m_time = 0.;

    GameObject *m_owner = nullptr;
};
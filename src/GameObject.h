#pragma once

#include <memory>
#include <functional>

#include "ResourceIdentifiers.h"
#include "Polygon.h"

class GameWorld;

enum class Multiplier
{
    SCATTER,
    ALIGN,
    SEEK,
    VELOCITY,
    AVOID
};

struct CollisionData
{
    sf::Vector2f separation_axis;
    float minimum_translation = -1;
    bool belongs_to_a = true;
    sf::Vector2f contact_point;
};

enum class ObjectType
{
    Enemy,
    Bullet,
    Missile,
    Bomb,
    Laser,
    Meteor,
    Heart,
    SpaceStation,
    Explosion,
    Player,
    Flag,
    Boss,
    Trigger,
    EMP,
    Count
};

enum class EffectType
{
    ParticleEmiter,
    AnimatedSprite,

};

enum class ObjectiveType
{
    ReachSpot,
    DestroyEnemy,
    Count
};

struct RigidBody
{
    float mass;
    float inertia;
    float angle_vel;
};

class GameObject
{

public:
    GameObject(GameWorld *world, TextureHolder &textures, ObjectType type);

    virtual void update(float dt) = 0;
    virtual void onCreation() = 0;
    virtual void onDestruction() {m_on_destruction_callback(m_id, m_type);}
    virtual void draw(sf::RenderTarget &target) = 0;
    virtual void onCollisionWith(GameObject &obj, CollisionData &c_data) = 0;
    virtual ~GameObject() {}

    void removeCollider();
    bool isBloomy() const;
    void kill();
    bool isDead() const;
    void updateAll(float dt);

    const sf::Vector2f &getPosition() const;
    void setPosition(sf::Vector2f new_position);
    void move(sf::Vector2f by);

    float getAngle() const;
    void setAngle(float angle);
    
    bool collides() const;
    Polygon &getCollisionShape();
    
    bool doesPhysics()const;
    RigidBody &getRigidBody();
    
    int getId() const;
    ObjectType getType() const;


    void setSize(sf::Vector2f size);

    void setDestructionCallback(std::function<void(int, ObjectType)> callback)
    {
        m_on_destruction_callback = callback;
    }

public:

    sf::Vector2f m_vel = {0, 0};
    int m_id;

protected:
    TextureHolder &m_textures;
    
    std::unique_ptr<Polygon> m_collision_shape = nullptr;
    std::unique_ptr<RigidBody> m_rigid_body = nullptr;
    
    sf::Vector2f m_pos;
    float m_angle = 0;
    
    GameWorld *m_world;
    
    bool m_is_dead = false;
    bool m_is_bloomy = false;
    
    sf::Vector2f m_size = {1, 1};

private:
    std::function<void(int, ObjectType)> m_on_destruction_callback = [](int, ObjectType){};

    ObjectType m_type;
};
#pragma once

#include "core.h"
#include <memory>
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

    ObjectType m_type;

protected:

    TextureHolder& m_textures;

    sf::Vector2f m_pos;
    GameWorld* m_world; //! won't be needed once I implement messenger class

    bool m_is_dead = false;
    bool m_is_bloomy = false;

    sf::Vector2f m_size = {1,1};

public:
    GameObject(GameWorld* world, TextureHolder& textures, ObjectType type);

    std::unique_ptr<Polygon> m_collision_shape = nullptr;
    std::unique_ptr<RigidBody> m_rigid_body = nullptr;

    float m_angle = 0;
    int m_id;
    sf::Vector2f m_vel = {0,0};

    virtual void update(float dt) = 0;
    virtual void onCreation() = 0;
    virtual void onDestruction() = 0;
    virtual void draw(sf::RenderTarget &target) = 0;
    virtual void onCollisionWith(GameObject& obj, CollisionData& c_data) = 0;
    virtual ~GameObject() {}

    void removeCollider();
    bool isBloomy()const;
    void kill();
    bool isDead()const;
    void updateAll(float dt);

    void move(sf::Vector2f by);
    bool collides() const;
    float getAngle() const;
    Polygon &getCollisionShape();
    int getId() const;
    ObjectType getType() const;

    const sf::Vector2f &getPosition() const;
    void setPosition(sf::Vector2f new_position);
    void setSize(sf::Vector2f size);
    void setAngle(float angle);
};
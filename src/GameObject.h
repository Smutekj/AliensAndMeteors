#pragma once

#include <memory>
#include <functional>

#include "Polygon.h"
#include "Components.h"

class GameWorld;
class TextureHolder;
class LayersHolder;
// class GameSystems;

constexpr int MAX_ENTITY_COUNT = 10000;

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
    utils::Vector2f separation_axis;
    float minimum_translation = -1;
    bool belongs_to_a = true;
    utils::Vector2f contact_point = {0, 0};
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

struct PlayerEntity;
namespace Collisions
{
    class CollisionSystem;
}

class GameObject
{

public:
    GameObject() {};
    GameObject(GameWorld *world, TextureHolder &textures,
               ObjectType type = ObjectType::Count, PlayerEntity *player = nullptr);
    GameObject(const GameObject &other) = default;
    GameObject(GameObject &&other) = default;
    GameObject &operator=(GameObject &other) = default;
    GameObject &operator=(GameObject &&other) = default;

    virtual ~GameObject() = default;

    virtual void update(float dt) {};
    virtual void onCreation() {};
    virtual void onDestruction() { m_on_destruction_callback(getId(), m_type); }
    virtual void draw(LayersHolder &target) {};
    virtual void onCollisionWith(GameObject &obj, CollisionData &c_data) {};

    void removeCollider();

    bool isRoot() const;

    void kill();
    bool isDead() const;

    void updateAll(float dt);

    const utils::Vector2f &getPosition() const;
    void setPosition(utils::Vector2f new_position);
    void move(utils::Vector2f by);

    float getAngle() const;
    void setAngle(float angle);

    bool collides() const;
    Polygon &getCollisionShape();

    bool doesPhysics() const;
    RigidBody &getRigidBody();

    int getId() const;
    ObjectType getType() const;

    void setSize(utils::Vector2f size);
    const utils::Vector2f &getSize() const;

    void setDestructionCallback(std::function<void(int, ObjectType)> callback);
    void addChild(GameObject* child);
    void removeChild(GameObject* child);

    bool isParentOf(GameObject* child) const;

public:
    utils::Vector2f m_vel = {0, 0};
    utils::Vector2f m_acc = {0, 0};
    float m_max_vel = 70.f;
    float m_max_acc = 250.f;
    
    int m_id;

    std::vector<GameObject*> m_children;
    GameObject *m_parent = nullptr;

    std::unordered_map<ObjectType, std::function<void(GameObject&, CollisionData&)>> m_collision_resolvers;

protected:
    TextureHolder *m_textures;

    std::shared_ptr<Polygon> m_collision_shape = nullptr;
    std::shared_ptr<RigidBody> m_rigid_body = nullptr;

    //! transform data
    utils::Vector2f m_pos;
    float m_angle = 0;
    utils::Vector2f m_size = {1, 1};

    GameWorld *m_world;

    bool m_is_dead = false;
    
private:
    std::function<void(int, ObjectType)> m_on_destruction_callback = [](int, ObjectType) {};

    ObjectType m_type;
};
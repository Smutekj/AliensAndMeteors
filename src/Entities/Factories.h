#pragma once

#include "../GameWorld.h"

#include "../SoundSystem.h"

enum class EnemyType
{
    LaserEnemy,
    LaserEnemyNoTarget,
    BombEnemy,
    ShooterEnemy,
    EnergyShooter,
    RocketEnemy
};

template <class Factory, class EntityT, class EntityIdentifier, class... Args>
class EntityFactory
{

public:
    EntityFactory(GameWorld &world) : m_world(world)
    {
    }

    virtual void registerCreators(TextureHolder &textures) = 0;

    GameObject &create2(EntityIdentifier id, utils::Vector2f pos, Args... args)
    {
        assert(isRegistered(id));
        auto &new_entity = m_world.addObject2<EntityT>();
        new_entity.setPosition(pos);
        return m_creators.at(id)(new_entity, args...);
    }

    bool isRegistered(EntityIdentifier id) const
    {
        return m_creators.contains(id);
    }

protected:
    std::unordered_map<
        EntityIdentifier,
        std::function<EntityT &(EntityT &, Args...)>>
        m_creators;

    GameWorld &m_world;
};

class EnemyFactory : public EntityFactory<EnemyFactory, Enemy, EnemyType>
{
public:
    EnemyFactory(GameWorld &world, TextureHolder &textures);
    virtual void registerCreators(TextureHolder &textures) override;
};

enum class MeteorType
{
    HardDestroyable,
    Hard,
    Soft,
    Exploding,
};
class MeteorFactory : public EntityFactory<MeteorFactory, Meteor, MeteorType>
{
public:
    MeteorFactory(GameWorld &world, TextureHolder &textures);
    virtual void registerCreators(TextureHolder &textures) override;
};

struct EnemySpec
{
    float avg_hp;
    float avg_speed;
    float avg_acc;
    float avg_shoot_cd;
    float avg_dmg;
};

class EnemyFactory2 : public EntityFactory<EnemyFactory, Enemy, EnemyType, EnemySpec>
{
public:
    EnemyFactory2(GameWorld &world, TextureHolder &textures);
    virtual void registerCreators(TextureHolder &textures) override;
};

class HomingProjectileFactory : public EntityFactory<HomingProjectileFactory, Bullet, ProjectileType, GameObject *>
{
public:
    HomingProjectileFactory(GameWorld &world, TextureHolder &textures);

    virtual void registerCreators(TextureHolder &textures) override;
};

class ExplosionFactory : public EntityFactory<ExplosionFactory, Explosion, AnimationId, float>
{
public:
    ExplosionFactory(GameWorld &world, TextureHolder &textures);

    virtual void registerCreators(TextureHolder &textures) override;
};

class ProjectileFactory : public EntityFactory<ProjectileFactory, Bullet, ProjectileType, ColorByte>
{
public:
    ProjectileFactory(GameWorld &world, TextureHolder &textures);

    virtual void registerCreators(TextureHolder &textures) override;

private:
    void addLaserCollider(Bullet &entity);
    void addCircleCollider(Bullet &entity);
    void killAfter(float delay, Bullet &bullet);

    ExplosionFactory m_boom_factory;
};

class LaserFactory : public EntityFactory<LaserFactory, Laser, LaserType, ColorByte>
{
public:
    LaserFactory(GameWorld &world, TextureHolder &textures);

    virtual void registerCreators(TextureHolder &textures) override;
};

class PickupFactory : public EntityFactory<PickupFactory, Heart, Pickup>
{
public:
    PickupFactory(GameWorld &world, TextureHolder &textures);

    virtual void registerCreators(TextureHolder &textures) override;
};

enum class WallType
{
    SolidWall,
    FireWall,
    ElectroWall,
    SpeedWall,
    StoneWall,
};

template <class EntityIdentifier, class... Args>
class GameObjectFactory
{

public:
    GameObjectFactory(GameWorld &world, ObjectType type) : m_world(world), m_type(type)
    {
    }

    virtual void registerCreators(TextureHolder &textures) = 0;
    virtual ~GameObjectFactory() {};

    GameObject &create2(EntityIdentifier id, utils::Vector2f pos, Args... args)
    {
        assert(isRegistered(id));
        auto &new_entity = m_world.addObject3(m_type);
        new_entity.setPosition(pos);
        return m_creators.at(id)(new_entity, args...);
    }

    bool isRegistered(EntityIdentifier id) const
    {
        return m_creators.contains(id);
    }

protected:
    std::unordered_map<
        EntityIdentifier,
        std::function<GameObject &(GameObject &, Args...)>>
        m_creators;

    GameWorld &m_world;
    ObjectType m_type;
};

class WallFactory : public GameObjectFactory<WallType, float, utils::Vector2f>
{
public:
    WallFactory(GameWorld &world, TextureHolder &textures);

    virtual void registerCreators(TextureHolder &textures) override;
    virtual ~WallFactory() override = default;

private:
    void addHolders(GameObject &wall, utils::Vector2f size, TextureHolder &textures);
};

//! for later

// struct GameObjectSpec
// {
//     utils::Vector2f pos;
//     float angle;
//     utils::Vector2f size;
//     float max_speed;
//     float max_acc;
//     ObjectType type;

//     using EntityTypeId = std::size_t;

// protected:
//     static EntityTypeId runtime_id;
// };

// struct EnemySpec2 : public GameObjectSpec
// {
//     EnemyType en_type;
//     float max_hp;
//     float vision_range;
//     float weapon_range;
//     float cooldown;
// };

// class FactoryBase
// {
// public:
//     virtual GameObject &create2(std::size_t id, GameObjectSpec &spec) = 0;

//     template <class IdType>
//     GameObject &create(IdType id, GameObjectSpec &spec)
//     {
//         create2(static_cast<std::size_t>(id), spec);
//     }
// };

// template <class IdType, class SpecType>
// class EntityFactory3 : public FactoryBase
// {

//     using FatoryFunc = std::function<GameObject &(GameObject &, SpecType &)>;

// public:
//     EntityFactory3(GameWorld &world, TextureHolder &textures) : m_world(world)
//     {
//         static_assert(std::is_base_of_v<GameObjectSpec, SpecType>);
//         registerCreators(textures);
//     }

//     virtual void registerCreators(TextureHolder &textures) = 0;

//     virtual GameObject &create2(std::size_t id, GameObjectSpec &spec) override
//     {
//         auto &new_entity = m_world.addObject3();
//         return m_creators.at(id)(new_entity, static_cast<SpecType &>(spec));
//     }

//     void registerCreator(IdType id, FactoryFunc creator)
//     {
//         m_creators[static_cast<std::size_t>(id)] = creator;
//     }

// protected:
//     GameWorld &m_world;

// private:
//     std::unordered_map<std::size_t, FactoryFunc> m_creators;
// };

// class EnemyFactory3 : public EntityFactory3<EnemyType, EnemySpec2>
// {
//     EnemyFactory3(GameWorld &world, TextureHolder &textures)
//         : EntityFactory3<EnemyType, EnemySpec>(world, textures)
//     {
//     }

//     virtual void registerCreators(TextureHolder &textures) override
//     {
//         registerCreator(EnemyType::BombEnemy,
//                         [](GameObject &enemy, EnemySpec &spec)
//                         {
//                             //! do something...
//                             return enemy;
//                         });
//     }
// };
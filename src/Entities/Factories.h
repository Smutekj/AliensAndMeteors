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

    EntityT &create2(EntityIdentifier id, utils::Vector2f pos, Args...args)
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

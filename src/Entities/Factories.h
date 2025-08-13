#pragma once

#include "../GameWorld.h"

enum class EnemyType
{
    LaserEnemy,
    BombEnemy,
    ShooterEnemy,
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

    EntityT &create2(EntityIdentifier id, utils::Vector2f pos, Args &&...args)
    {
        assert(isRegistered(id));
        auto &new_entity = m_world.addObject2<EntityT>();
        new_entity.setPosition(pos);
        return m_creators.at(id)(new_entity, std::forward<Args>(args)...);
    }

    bool isRegistered(EntityIdentifier id) const
    {
        return m_creators.contains(id);
    }

protected:
    std::unordered_map<
        EntityIdentifier,
        std::function<EntityT &(EntityT &, Args &&...)>>
        m_creators;

    GameWorld &m_world;
};

class EnemyFactory : public EntityFactory<EnemyFactory, Enemy, EnemyType>
{
public:
    EnemyFactory(GameWorld &world, TextureHolder &textures)
        : EntityFactory<EnemyFactory, Enemy, EnemyType>(world)
    {
        registerCreators(textures);
    }
    virtual void registerCreators(TextureHolder &textures) override
    {
        auto laser_creator = [this, textures](Enemy &enemy) -> Enemy &
        {
            HealthComponent h_comp = {.hp = 40., .max_hp = 40.};
            TargetComponent t_comp = {.p_target = m_world.m_player, .targetting_strength = 200.f};
            m_world.m_systems.addEntityDelayed(enemy.getId(), BoidComponent{}, AvoidMeteorsComponent{},
                                               h_comp, t_comp, LaserAIComponent{});
            enemy.m_sprite.setTexture(*textures.get("EnemyLaser"));
            return enemy;
        };
        auto shooter_creator = [this, textures](Enemy &enemy) -> Enemy &
        {
            HealthComponent h_comp = {.hp = 20., .max_hp = 20.};
            TargetComponent t_comp = {.p_target = m_world.m_player, .targetting_strength = 1000.};
            ShootPlayerAIComponent s_comp = {.cooldown = 3.};
            m_world.m_systems.addEntityDelayed(enemy.getId(), BoidComponent{}, AvoidMeteorsComponent{},
                                               h_comp, t_comp, s_comp);
            enemy.m_sprite.setTexture(*textures.get("EnemyShip"));
            return enemy;
        };

        m_creators[EnemyType::LaserEnemy] = laser_creator;
        m_creators[EnemyType::ShooterEnemy] = shooter_creator;
    }
};

class ProjectileFactory : public EntityFactory<ProjectileFactory, Bullet, ProjectileType>
{
public:
    ProjectileFactory(GameWorld &world, TextureHolder &textures) : EntityFactory(world)
    {
        registerCreators(textures);
    }

    virtual void registerCreators(TextureHolder &textures) override
    {
        auto fire_bullet_creator = [this](Bullet &bullet) -> Bullet &
        {
            bullet.setBulletType(BulletType::Fire);
            bullet.setSize({4});
            return bullet;
        };

        auto homing_bullet_creatoer = [this](Bullet &bullet) -> Bullet &
        {
            TargetComponent t_comp = {.p_target = m_world.m_player, .targetting_strength = 100.f};
            m_world.m_systems.addEntityDelayed(bullet.getId(), t_comp);
            bullet.setSize({6});
            bullet.m_vel = bullet.m_max_vel * (m_world.m_player->getPosition() - bullet.getPosition());
            bullet.setBulletType(BulletType::Fire);
            return bullet;
        };

        m_creators[ProjectileType::FireBullet] = fire_bullet_creator;
        m_creators[ProjectileType::HomingFireBullet] = homing_bullet_creatoer;
    }
};

class LaserFactory : public EntityFactory<LaserFactory, Laser, LaserType, ColorByte>
{
public:
    LaserFactory(GameWorld &world, TextureHolder &textures)
        : EntityFactory<LaserFactory, Laser, LaserType, ColorByte>(world)
    {
        registerCreators(textures);
    }

    virtual void registerCreators(TextureHolder &textures) override
    {
        auto basic_creator = [this, textures](Laser &laser, ColorByte color) -> Laser &
        {
            laser.m_laser_color = color;
            // m_world.m_systems.addEntityDelayed(laser.getId());
            return laser;
        };

        m_creators[LaserType::Basic] = basic_creator;
    }
};

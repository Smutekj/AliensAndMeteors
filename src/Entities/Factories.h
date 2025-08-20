#pragma once

#include "../GameWorld.h"

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
        auto laser_wtf_creator = [this, &textures](Enemy &enemy) -> Enemy &
        {
            HealthComponent h_comp = {.max_hp = 40.};
            m_world.m_systems.addEntityDelayed(enemy.getId(), h_comp);
            enemy.m_sprite.setTexture(*textures.get("EnemyLaser"));
            enemy.m_max_vel = 100;
            return enemy;
        };
        auto laser_creator = [this, &textures](Enemy &enemy) -> Enemy &
        {
            HealthComponent h_comp = {.max_hp = 40.};
            TargetComponent t_comp = {.p_target = m_world.m_player, .targetting_strength = 200.f};
            m_world.m_systems.addEntityDelayed(enemy.getId(), BoidComponent{}, AvoidMeteorsComponent{},
                                               h_comp, t_comp, LaserAIComponent{});
            enemy.m_sprite.setTexture(*textures.get("EnemyLaser"));
            enemy.m_max_vel = 50;
            return enemy;
        };
        auto shooter_creator = [this, &textures](Enemy &enemy) -> Enemy &
        {
            HealthComponent h_comp = { .max_hp = 20.};
            TargetComponent t_comp = {.p_target = m_world.m_player, .targetting_strength = 1000.};
            ShootPlayerAIComponent s_comp = {.cooldown = 3.};
            SpriteComponent sprite_comp = {.layer_id = "Unit", .sprite = Sprite{*textures.get("EnemyShip")}};
            m_world.m_systems.addEntityDelayed(enemy.getId(), BoidComponent{}, AvoidMeteorsComponent{},
                                               h_comp, t_comp, s_comp, sprite_comp);
            // enemy.m_sprite.setTexture(*textures.get("EnemyShip"));
            return enemy;
        };
        auto energy_shooter_creator = [this, &textures](Enemy &enemy) -> Enemy &
        {
            HealthComponent h_comp = {.max_hp = 40.};
            TargetComponent t_comp = {.p_target = m_world.m_player, .targetting_strength = 1000.};
            ShootPlayerAIComponent s_comp = {.cooldown = 3., .projectile_type = ProjectileType::EnergyBullet};
            SpriteComponent sprite_comp = {.layer_id = "Unit", .sprite = Sprite{*textures.get("EnemyBomber")}};
            m_world.m_systems.addEntityDelayed(enemy.getId(), BoidComponent{}, AvoidMeteorsComponent{},
                                               h_comp, t_comp, s_comp, sprite_comp);
            enemy.m_max_vel = 80.f; 
            return enemy;
        };

        m_creators[EnemyType::LaserEnemyNoTarget] = laser_wtf_creator;
        m_creators[EnemyType::LaserEnemy] = laser_creator;
        m_creators[EnemyType::ShooterEnemy] = shooter_creator;
        m_creators[EnemyType::EnergyShooter] = energy_shooter_creator;
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
        auto fire_bullet_creator = [this, &textures](Bullet &bullet) -> Bullet &
        {
            SpriteComponent s_comp = {.layer_id = "Unit", .shader_id="fireBolt", .sprite = Sprite{*textures.get("EnergyBullet")}};
            bullet.setBulletType(BulletType::Fire);
            bullet.setSize({4});
            bullet.m_max_vel = 300.f;
            // utils::Vector2f shoot_dir = shooter->m_vel / utils::norm(shooter->m_vel);
            // bullet.m_vel = shoot_dir * bullet.m_max_vel;
            // bullet.setPosition(bullet.getPosition() + shoot_dir*5);
            
            return bullet;
        };
        auto electro_bullet_creator = [this, &textures](Bullet &bullet, GameObject* owner = nullptr) -> Bullet &
        {
            SpriteComponent s_comp = {.layer_id = "Unit", .shader_id="lightningBolt", .sprite = Sprite{*textures.get("EnergyBullet")}};
            bullet.setBulletType(BulletType::Lightning);
            bullet.setSize({4});
            bullet.m_max_vel = 300.f;
            m_world.m_systems.addEntityDelayed(bullet.getId(), s_comp);

            return bullet;
        };
        auto energy_bullet_creator = [&textures, this](Bullet &bullet, GameObject* owner = nullptr) -> Bullet &
        {
            bullet.setBulletType(BulletType::Fire);
            bullet.setSize({4});
            bullet.m_max_vel = 200.f;
            bullet.m_collision_resolvers[ObjectType::Player] = [&bullet](GameObject& obj, CollisionData& c_data)
            {
                auto* player = dynamic_cast<PlayerEntity*>(&obj);
                assert(player);
                player->m_fuel -= 5.;
                player->speed *= 0.95f;
                bullet.kill();
                // player->booster = BoosterState::Disabled;
            };
            SpriteComponent s_comp = {.layer_id = "Unit", .sprite = Sprite{*textures.get("EnergyBullet")}};
            m_world.m_systems.addEntityDelayed(bullet.getId(), s_comp);
            
            return bullet;
        };
        
        auto homing_bullet_creator = [this, &textures](Bullet &bullet, GameObject* owner = nullptr) -> Bullet &
        {
            SpriteComponent s_comp = {.layer_id = "Unit", .shader_id="fireBolt", .sprite = Sprite{*textures.get("EnergyBullet")}};
            bullet.setBulletType(BulletType::Fire);
            bullet.setSize({6});

            TargetComponent t_comp = {.p_target = m_world.m_player, .targetting_strength = 100.f};
            m_world.m_systems.addEntityDelayed(bullet.getId(), t_comp);
            bullet.m_max_vel = 150.f;
            bullet.m_vel = bullet.m_max_vel * (m_world.m_player->getPosition() - bullet.getPosition());
            return bullet;
        };

        m_creators[ProjectileType::FireBullet] = fire_bullet_creator;
        m_creators[ProjectileType::HomingFireBullet] = homing_bullet_creator;
        m_creators[ProjectileType::EnergyBullet] = energy_bullet_creator;
        m_creators[ProjectileType::ElectroBullet] = electro_bullet_creator;

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
class PickupFactory : public EntityFactory<PickupFactory, Heart, Pickup>
{
public:
PickupFactory(GameWorld &world, TextureHolder &textures)
        : EntityFactory<PickupFactory, Heart, Pickup>(world)
    {
        registerCreators(textures);
    }

    virtual void registerCreators(TextureHolder &textures) override
    {

        m_creators[Pickup::Heart] = [this, &textures](Heart& pickup) -> Heart&
        {  
            SpriteComponent sp_comp = {.layer_id = "Unit", .sprite = {*textures.get("Heart")}};
            m_world.m_systems.addEntityDelayed(pickup.getId(), sp_comp);
            pickup.m_collision_resolvers[ObjectType::Player] = [this, &pickup](GameObject& obj, auto& c_data)
            {
                m_world.m_systems.get<HealthComponent>(obj.getId()).hp += 5;
                pickup.kill();
            };
            return pickup;
        };
        m_creators[Pickup::Fuel] = [this, &textures](Heart& pickup) -> Heart&
        {  
            SpriteComponent sp_comp = {.layer_id = "Unit", .sprite = {*textures.get("Fuel")}};
            m_world.m_systems.addEntityDelayed(pickup.getId(), sp_comp);
            pickup.m_collision_resolvers[ObjectType::Player] = [this, &pickup](GameObject& obj, auto& c_data)
            {
                auto p_player = dynamic_cast<PlayerEntity*>(&obj);
                assert(p_player);
                p_player->m_fuel = std::min(p_player->m_fuel + 5, p_player->m_max_fuel); 
                pickup.kill();
            };
            pickup.m_collision_resolvers[ObjectType::Bullet] = [this, &pickup](GameObject& obj, auto& c_data)
            {
                pickup.kill();
                obj.kill();
            };
            return pickup;
        };
        m_creators[Pickup::Money] = [this, &textures](Heart& pickup) -> Heart&
        {  
            SpriteComponent sp_comp = {.layer_id = "Unit", .sprite = {*textures.get("Coin")}};
            m_world.m_systems.addEntityDelayed(pickup.getId(), sp_comp);
            pickup.m_collision_resolvers[ObjectType::Player] = [this, &pickup](GameObject& obj, auto& c_data)
            {
                auto p_player = dynamic_cast<PlayerEntity*>(&obj);
                assert(p_player);
                p_player->m_money += 5; 
                pickup.kill();
            };
            return pickup;
        };
    }
};

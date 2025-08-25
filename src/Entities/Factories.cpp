#include "Factories.h"

#include "../Utils/RandomTools.h"

EnemyFactory::EnemyFactory(GameWorld &world, TextureHolder &textures)
    : EntityFactory<EnemyFactory, Enemy, EnemyType>(world)
{
    registerCreators(textures);
}

void EnemyFactory::registerCreators(TextureHolder &textures)
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
        HealthComponent h_comp = {.max_hp = 20.};
        TargetComponent t_comp = {.p_target = m_world.m_player, .targetting_strength = 1000.};
        ShootPlayerAIComponent s_comp = {.cooldown = 1.};
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

HomingProjectileFactory::HomingProjectileFactory(GameWorld &world, TextureHolder &textures) : EntityFactory(world)
{
    registerCreators(textures);
}

void HomingProjectileFactory::registerCreators(TextureHolder &textures)
{
    auto homing_bullet_creator = [this, &textures](Bullet &bullet, GameObject *target) -> Bullet &
    {
        TargetComponent t_comp = {.p_target = target, .targetting_strength = 100.f};
        m_world.m_systems.addEntityDelayed(bullet.getId(), t_comp);
        bullet.m_max_vel = 150.f;
        bullet.m_vel = bullet.m_max_vel * (target->getPosition() - bullet.getPosition());
        bullet.setSize({6});
        return bullet;
    };

    auto electro_bullet_creator = [this, &textures, homing_bullet_creator](Bullet &bullet, GameObject *target) -> Bullet &
    {
        SpriteComponent s_comp = {.layer_id = "Unit", .shader_id = "lightningBolt", .sprite = Sprite{*textures.get("EnergyBullet")}};
        TargetComponent t_comp = {.p_target = target, .targetting_strength = 100.f};
        bullet.m_max_vel = 150.f;
        bullet.m_vel = bullet.m_max_vel * (target->getPosition() - bullet.getPosition());
        bullet.setSize({5});
        m_world.m_systems.addEntityDelayed(bullet.getId(), t_comp, s_comp);
        return bullet;
    };

    auto energy_bullet_creator = [&textures, this](Bullet &bullet, GameObject *target) -> Bullet &
    {
        SpriteComponent s_comp = {.layer_id = "Unit", .sprite = Sprite{*textures.get("EnergyBullet")}};
        TargetComponent t_comp = {.p_target = target, .targetting_strength = 100.f};
        bullet.m_max_vel = 150.f;
        bullet.m_vel = bullet.m_max_vel * (target->getPosition() - bullet.getPosition());
        bullet.setSize({5});
        m_world.m_systems.addEntityDelayed(bullet.getId(), t_comp, s_comp);
        bullet.m_collision_resolvers[ObjectType::Player] = [&bullet](GameObject &obj, CollisionData &c_data)
        {
            auto *player = dynamic_cast<PlayerEntity *>(&obj);
            assert(player);
            player->m_fuel -= 5.;
            player->speed *= 0.95f;
            bullet.kill();
            // player->booster = BoosterState::Disabled;
        };
        return bullet;
    };

    m_creators[ProjectileType::HomingFireBullet] = homing_bullet_creator;
    m_creators[ProjectileType::EnergyBullet] = energy_bullet_creator;
    m_creators[ProjectileType::ElectroBullet] = electro_bullet_creator;
}

ExplosionFactory::ExplosionFactory(GameWorld &world, TextureHolder &textures)
    : EntityFactory<ExplosionFactory, Explosion, AnimationId, float>(world)
{
    registerCreators(textures);
}

void ExplosionFactory::registerCreators(TextureHolder &textures)
{

    auto creator_skelet = [this, &textures](Explosion &boom, float duration, AnimationId id) -> Explosion &
    {
        AnimationComponent anim;
        anim.id = id;
        anim.cycle_duration = duration;

        TimedEventComponent timer;
        timer.addEvent(TimedEvent{duration,
                                  [&boom](float t, int count)
                                  { boom.kill(); },
                                  1});

        m_world.m_systems.addEntityDelayed(boom.getId(), anim, timer);
        return boom;
    };

    m_creators[AnimationId::PurpleExplosion] = [this, &textures, creator_skelet](Explosion &boom, float duration) -> Explosion &
    {
        return creator_skelet(boom, duration, AnimationId::PurpleExplosion);
    };
    m_creators[AnimationId::BlueExplosion] = [this, &textures, creator_skelet](Explosion &boom, float duration) -> Explosion &
    {
        return creator_skelet(boom, duration, AnimationId::PurpleExplosion);
    };
}

ProjectileFactory::ProjectileFactory(GameWorld &world, TextureHolder &textures)
    : EntityFactory(world),
      m_boom_factory(world, textures)
{
    registerCreators(textures);
}

void ProjectileFactory::addLaserCollider(Bullet &entity)
{
    CollisionComponent c_comp;
    c_comp.type = ObjectType::Bullet;
    c_comp.shape.convex_shapes.emplace_back(4);
    m_world.m_systems.addEntityDelayed(entity.getId(), c_comp);
}

void ProjectileFactory::addCircleCollider(Bullet &entity)
{
    CollisionComponent c_comp;
    c_comp.type = ObjectType::Bullet;
    c_comp.shape.convex_shapes.emplace_back(8);
    m_world.m_systems.addEntityDelayed(entity.getId(), c_comp);
}

void ProjectileFactory::registerCreators(TextureHolder &textures)
{

    auto fire_bullet_creator = [this, &textures](Bullet &bullet, ColorByte c = {255, 255, 255, 255}) -> Bullet &
    {
        bullet.setSize({4});
        bullet.m_max_vel = 300.f;

        SpriteComponent s_comp = {.layer_id = "Unit", .shader_id = "fireBolt", .sprite = Sprite{*textures.get("EnergyBullet")}};
        m_world.m_systems.addEntityDelayed(bullet.getId(), s_comp);
        addCircleCollider(bullet);
        killAfter(10.f, bullet);
        return bullet;
    };
    auto electro_bullet_creator = [this, &textures](Bullet &bullet,  ColorByte c = {255, 255, 255, 255}) -> Bullet &
    {
        bullet.setSize({4});
        bullet.m_max_vel = 300.f;

        SpriteComponent s_comp = {.layer_id = "Unit", .shader_id = "lightningBolt", .sprite = Sprite{*textures.get("EnergyBullet")}};
        m_world.m_systems.addEntityDelayed(bullet.getId(), s_comp);
        addCircleCollider(bullet);
        killAfter(10.f, bullet);
        return bullet;
    };

    auto energy_bullet_creator = [&textures, this](Bullet &bullet,  ColorByte c = {255, 255, 255, 255}) -> Bullet &
    {
        bullet.setSize({4});
        bullet.m_max_vel = 200.f;
        bullet.m_collision_resolvers[ObjectType::Player] = [&bullet](GameObject &obj, CollisionData &c_data)
        {
            auto *player = dynamic_cast<PlayerEntity *>(&obj);
            assert(player);
            player->m_fuel -= 5.;
            player->speed *= 0.95f;
            bullet.kill();
            // player->booster = BoosterState::Disabled;
        };

        SpriteComponent s_comp = {.layer_id = "Unit", .sprite = Sprite{*textures.get("EnergyBullet")}};
        m_world.m_systems.addEntityDelayed(bullet.getId(), s_comp);
        addCircleCollider(bullet);
        killAfter(10.f, bullet);
        return bullet;
    };

    auto homing_bullet_creator = [this, &textures](Bullet &bullet,  ColorByte c = {255, 255, 255, 255}) -> Bullet &
    {
        SpriteComponent s_comp = {.layer_id = "Unit", .shader_id = "fireBolt", .sprite = Sprite{*textures.get("EnergyBullet")}};
        TargetComponent t_comp = {.p_target = m_world.m_player, .targetting_strength = 100.f};
        m_world.m_systems.addEntityDelayed(bullet.getId(), t_comp, s_comp);
        addCircleCollider(bullet);

        bullet.m_max_vel = 150.f;
        bullet.m_vel = bullet.m_max_vel * (m_world.m_player->getPosition() - bullet.getPosition());
        bullet.setSize({6});

        killAfter(10.f, bullet);

        return bullet;
    };

    auto laser_bullet_creator = [this, &textures](Bullet &bullet, ColorByte c = {255, 255, 255, 255}) -> Bullet &
    {
        Sprite sprite{*textures.get("EnergyBullet")};
        sprite.setColor(c);
        SpriteComponent s_comp = {.layer_id = "Bloom", .shader_id = "laser", .sprite = sprite};
        bullet.setSize({5, 1});

        m_world.m_systems.addEntityDelayed(bullet.getId(), s_comp);
        bullet.m_max_vel = 300.f;
        addCircleCollider(bullet);

        
        std::vector<SoundID> laser_sounds = {SoundID::Laser1, SoundID::Laser2, SoundID::Laser3};
        SoundSystem::play(randomValue(laser_sounds), utils::dist(bullet.getPosition(), m_world.m_player->getPosition()));

        killAfter(10.f, bullet);

        return bullet;
    };
    auto rocket_creator = [this, &textures](Bullet &bullet, ColorByte c = {255, 255, 255, 255}) -> Bullet &
    {
        Sprite sprite{*textures.get("Missile")};
        SpriteComponent s_comp = {.layer_id = "Unit", .sprite = sprite};
        bullet.setSize({10, 3});
        bullet.m_collision_resolvers[ObjectType::Meteor] = [this, &bullet](GameObject &obj, auto &c_data)
        {
            auto &boom = m_boom_factory.create2(AnimationId::PurpleExplosion, bullet.getPosition(), 0.5f);
            boom.m_max_explosion_radius = (obj.getSize().x * 0.5f);
            SoundSystem::play(SoundID::Explosion1);
            bullet.kill();
            obj.kill();
        };
        bullet.m_max_vel = 200.f;
        addCircleCollider(bullet);

        //! create the exhaust tail
        auto &tail = m_world.addObject3(ObjectType::Count);
        ParticleComponent parts = {.layer_id = "Bloom", .shader_id = "Exhaust"};

        auto particles = std::make_unique<Particles>(100);
        // particles->setTexture(*textures.get("FireNoise"));
        // particles->setShader("laser");
        particles->setPeriod(0.03f);
        particles->setLifetime(3.f);
        particles->setInitColor({5.f, 0.1f, 0.f, 1.f});
        particles->setFinalColor({10.f, 10.f, 2.f, 1.f});
        particles->setEmitter([&bullet](utils::Vector2f pos) -> Particle
                              {
                Particle p;
                auto motion_dir = utils::angle2dir(bullet.getAngle());
                p.pos = bullet.getPosition() + motion_dir*pos.x;
                utils::Vector2f perprendicular_dir = {motion_dir.y, -motion_dir.x};
                p.vel = bullet.m_vel* 0.1f + perprendicular_dir * randf(-5.f, 5.f); 
                p.scale = {3.f, 3.f};
                p.life_time = 10.f;
                return p; });
        particles->setUpdater([&bullet](Particle &p, float dt)
                              {
                                  p.pos += p.vel * dt;
                                  p.angle += 60.f * dt;
                              }); 
        particles->setSpawnPos({-bullet.getSize().x/2., 0.f});
        parts.particles = std::move(particles);
        m_world.m_systems.addEntityDelayed(bullet.getId(), s_comp, parts);

        killAfter(10.f, bullet);

        std::vector<SoundID> rocket_sounds = {SoundID::Rocket1, SoundID::Rocket2, SoundID::Rocket3, SoundID::Rocket4};
        SoundSystem::play(randomValue(rocket_sounds), utils::dist(bullet.getPosition(), m_world.m_player->getPosition())/2.f);

        return bullet;
    };

    m_creators[ProjectileType::FireBullet] = fire_bullet_creator;
    m_creators[ProjectileType::HomingFireBullet] = homing_bullet_creator;
    m_creators[ProjectileType::EnergyBullet] = energy_bullet_creator;
    m_creators[ProjectileType::ElectroBullet] = electro_bullet_creator;
    m_creators[ProjectileType::LaserBullet] = laser_bullet_creator;
    m_creators[ProjectileType::Rocket] = rocket_creator;
}

void ProjectileFactory::killAfter(float delay, Bullet &bullet)
{
    TimedEventComponent timer;
    timer.addEvent({delay, [&bullet](float t, int n)
                    { bullet.kill(); }, 1});
    m_world.m_systems.addEntity(bullet.getId(), timer);
}

LaserFactory::LaserFactory(GameWorld &world, TextureHolder &textures)
    : EntityFactory<LaserFactory, Laser, LaserType, ColorByte>(world)
{
    registerCreators(textures);
}

void LaserFactory::registerCreators(TextureHolder &textures)
{
    auto basic_creator = [this, textures](Laser &laser, ColorByte color = {255, 255, 255, 255}) -> Laser &
    {
        laser.m_laser_color = color;
        // m_world.m_systems.addEntityDelayed(laser.getId());
        return laser;
    };

    m_creators[LaserType::Basic] = basic_creator;
}
PickupFactory::PickupFactory(GameWorld &world, TextureHolder &textures)
    : EntityFactory<PickupFactory, Heart, Pickup>(world)
{
    registerCreators(textures);
}

void PickupFactory::registerCreators(TextureHolder &textures)
{

    m_creators[Pickup::Heart] = [this, &textures](Heart &pickup) -> Heart &
    {
        SpriteComponent sp_comp = {.layer_id = "Unit", .sprite = {*textures.get("Heart")}};
        m_world.m_systems.addEntityDelayed(pickup.getId(), sp_comp);
        pickup.m_collision_resolvers[ObjectType::Player] = [this, &pickup](GameObject &obj, auto &c_data)
        {
            m_world.m_systems.get<HealthComponent>(obj.getId()).hp += 5;
            pickup.kill();
        };
        return pickup;
    };
    m_creators[Pickup::Shield] = [this, &textures](Heart &pickup) -> Heart &
    {
        SpriteComponent sp_comp = {.layer_id = "Unit", .sprite = {*textures.get("ShieldPickup")}};
        m_world.m_systems.addEntityDelayed(pickup.getId(), sp_comp);
        pickup.m_collision_resolvers[ObjectType::Player] = [this, &pickup](GameObject &obj, auto &c_data)
        {
            static_cast<PlayerEntity&>(obj).max_shield_hp = 20;
            pickup.kill();
        };
        return pickup;
    };
    m_creators[Pickup::Fuel] = [this, &textures](Heart &pickup) -> Heart &
    {
        SpriteComponent sp_comp = {.layer_id = "Unit", .sprite = {*textures.get("Fuel")}};
        m_world.m_systems.addEntityDelayed(pickup.getId(), sp_comp);
        pickup.m_collision_resolvers[ObjectType::Player] = [this, &pickup](GameObject &obj, auto &c_data)
        {
            auto p_player = dynamic_cast<PlayerEntity *>(&obj);
            assert(p_player);
            p_player->m_fuel = std::min(p_player->m_fuel + 5, p_player->m_max_fuel);
            pickup.kill();
        };
        pickup.m_collision_resolvers[ObjectType::Bullet] = [this, &pickup](GameObject &obj, auto &c_data)
        {
            pickup.kill();
            obj.kill();
        };
        return pickup;
    };
    m_creators[Pickup::Money] = [this, &textures](Heart &pickup) -> Heart &
    {
        SpriteComponent sp_comp = {.layer_id = "Unit", .sprite = {*textures.get("Coin")}};
        m_world.m_systems.addEntityDelayed(pickup.getId(), sp_comp);
        pickup.m_collision_resolvers[ObjectType::Player] = [this, &pickup](GameObject &obj, auto &c_data)
        {
            auto p_player = dynamic_cast<PlayerEntity *>(&obj);
            assert(p_player);
            p_player->m_money += 5;
            pickup.kill();
        };
        return pickup;
    };
}
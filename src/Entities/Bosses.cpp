#include "Bosses.h"

#include "../GameWorld.h"
#include "../Utils/RandomTools.h"

Boss1::Boss1(GameWorld *world, TextureHolder &textures, PlayerEntity *player)
    : p_player(player),
      m_projectile_factory(*world, textures),
      m_laser_factory(*world, textures),
      m_enemy_factory(*world, textures),
      GameObject(world, textures, ObjectType::Boss)
{
    m_size = {398 / 2, 230 / 2};
}

void Boss1::update(float dt)
{

    if (m_state == State::ShootingLasers)
    {
        //! periodic up-down motion
        m_motion_time += dt;
        float motion_frequency = 2.f * 3.14f / m_motion_period;
        float motion_amplitude = (m_size.y * 0.1f);
        m_pos.y += dt * motion_amplitude * motion_frequency * std::cos(m_motion_time * motion_frequency);
    }
    else if (m_state == State::ShootingBigLaser)
    {
        float motion_frequency = 2.f * 3.14f / m_motion_period;
        float motion_amplitude = (m_size.y * 0.5f);
        m_pos.y += dt * motion_amplitude * motion_frequency * std::cos(m_motion_time * motion_frequency);
    }
}

void Boss1::changeState(State target_state)
{
    if (m_on_state_change.contains(target_state))
    {
        m_on_state_change.at(target_state)();
    }
    m_state = target_state;
}

void Boss1::deActivateShield()
{
    assert(shield_id != -1);
    m_world->get(shield_id)->kill();
    shield_id = -1;
}

void Boss1::activateShield()
{
    auto &shield = m_world->addObject3(ObjectType::Shield);
    Polygon shield_collider = {32};
    CollisionComponent c_comp;
    c_comp.type = ObjectType::Shield;
    c_comp.shape.convex_shapes = {shield_collider};
    m_world->m_systems.addEntity(shield.getId(), c_comp);

    shield.setSize(m_size * 0.69f);
    shield.setAngle(90);
    addChild(&shield);
    shield_id = shield.getId();
}

void Boss1::onCreation()
{
    Polygon left_wing;
    left_wing.points = {{0.2, -0.3}, {0.5, 0.16}, {0.2, 0.14}};
    left_wing.setScale(m_size / 2.);
    Polygon right_wing;
    right_wing.points = {{-0.2, -0.3}, {-0.5, 0.16}, {-0.2, 0.14}};
    right_wing.setScale(m_size / 2.);
    CollisionComponent l_comp;
    CollisionComponent r_comp;
    l_comp.type = ObjectType::Boss;
    r_comp.type = ObjectType::Boss;
    l_comp.shape.convex_shapes = {left_wing};
    r_comp.shape.convex_shapes = {right_wing};
    auto &left_weapon = m_world->addObject2<Weapon>();
    auto &right_weapon = m_world->addObject2<Weapon>();
    m_world->m_systems.addEntity(left_weapon.getId(), l_comp);
    m_world->m_systems.addEntity(right_weapon.getId(), r_comp);
    left_weapon.setSize(m_size);
    right_weapon.setSize(m_size);
    left_weapon.setAngle(90);
    right_weapon.setAngle(90);
    addChild(&left_weapon);
    addChild(&right_weapon);

    activateShield();

    setAngle(90);

    Polygon hull = {4};
    hull.points = {{0.2, 0.5}, {0.2, -0.5}, {-0.2, -0.5}, {-0.2, 0.5}};
    hull.setScale(m_size / 2.);
    CollisionComponent c_comp;
    c_comp.type = ObjectType::Boss;
    c_comp.shape.convex_shapes = {hull};

    HealthComponent health = {.max_hp = 400};

    AnimationComponent anim;
    anim.id = AnimationId::Shield;
    anim.cycle_duration = 0.69;

    TimedEventComponent timer;
    m_world->m_systems.addEntity(getId(), timer, c_comp, health, anim);

    auto shoot_laser_at = [this](utils::Vector2f pos, utils::Vector2f target,
                                 utils::Vector2f offset, float width)
    {
        auto &laser = m_laser_factory.create2(LaserType::Basic, pos, {255, 10, 23});
        laser.setAngle(utils::dir2angle(target - pos - offset));
        addChild(&laser);

        laser.m_rotates_with_owner = false;
        int direction = 2 * (rand() % 2) - 1;
        laser.m_updater = [&laser, direction](float dt)
        {
            laser.setAngle(laser.getAngle() + direction * 10. * dt);
        };
        laser.m_offset = offset;
        laser.m_max_width = width;
        laser.m_max_length = 500.;
        laser.m_life_time = 3.;
        laser.m_max_dmg = 0.5f;
    };

    auto shoot_gun_at = [this](ProjectileType type, utils::Vector2f target, utils::Vector2f from, float speed, float size)
    {
        auto &bullet = m_projectile_factory.create2(type, from);
        bullet.setSize(size);
        auto dr = target - from;
        bullet.m_vel = dr / utils::norm(dr) * speed;
        bullet.m_collision_resolvers[ObjectType::Boss] = [id = getId()](auto &obj, auto &c_data)
        {
            if (id == obj.getId())
            {
                return; //! no collisions with boss
            }
        };
    };

    auto init_shooting_lasers = [this, shoot_laser_at]()
    {
        auto shoot_laser_middle = [this, shoot_laser_at](float t, int count)
        {
            auto target_pos = p_player->getPosition();
            shoot_laser_at(m_pos, target_pos, {0, 0}, 20.);
        };
        auto shoot_laser_upper = [this, shoot_laser_at](float t, int count)
        {
            auto target_pos = p_player->getPosition() + p_player->m_vel * 0.2;
            shoot_laser_at(m_pos, target_pos, {0, m_size.y * 0.3}, 4.);
        };
        auto shoot_laser_lower = [this, shoot_laser_at](float t, int count)
        {
            auto target_pos = p_player->getPosition();
            target_pos.y = std::min(target_pos.y, m_pos.y);
            shoot_laser_at(m_pos, target_pos, {0, -m_size.y * 0.3}, 4.f);
        };
        m_vel = {-1, 0};

        auto &timer = m_world->m_systems.get<TimedEventComponent>(getId());
        timer.addEvent({2.f, shoot_laser_lower, 3});
        timer.addEvent({5.f, shoot_laser_middle, 2});
        timer.addEvent({1.5f, shoot_laser_upper, 4});
        timer.addEvent({10.0f, [this](float t, int count)
                        { changeState(State::ShootingGuns); }, 1});
    };

    auto init_big_laser = [this, shoot_laser_at]()
    {
        auto shoot_laser_middle = [this, shoot_laser_at](float t, int count)
        {
            auto target_pos = p_player->getPosition() + p_player->m_vel * 0.2;
            shoot_laser_at(m_pos, target_pos, {0, 0}, 20.);
        };

        m_vel = {-2, 0};
        auto &timer = m_world->m_systems.get<TimedEventComponent>(getId());
        timer.addEvent({3.f, shoot_laser_middle, 2});
        timer.addEvent({7.f, [this](float t, int count)
                        { changeState(State::ShootingGuns); }, 1});
    };
    auto init_gun_shooting = [this, shoot_gun_at]()
    {
        auto shoot_left_gun = [this, shoot_gun_at](float t, int count)
        {
            auto target_pos = p_player->getPosition() + p_player->m_vel * 1.f;
            shoot_gun_at(ProjectileType::FireBullet, target_pos, m_pos - utils::Vector2f{m_size.x * 0.2f, m_size.y * 0.4f}, 130, 5);
        };
        auto shoot_right_gun = [this, shoot_gun_at](float t, int count)
        {
            auto target_pos = p_player->getPosition() + p_player->m_vel * 0.1f;
            shoot_gun_at(ProjectileType::ElectroBullet, target_pos, m_pos + utils::Vector2f{-m_size.x * 0.2f, m_size.y * 0.4f}, 130, 5);
        };
        auto shoot_middle_gun = [this, shoot_gun_at](float t, int count)
        {
            auto target_pos = p_player->getPosition();
            shoot_gun_at(ProjectileType::HomingFireBullet, target_pos, m_pos + utils::Vector2f{-m_size.x * 0.2f, 0.f}, 90, 10);
        };

        m_vel = {0, 0};
        auto &timer = m_world->m_systems.get<TimedEventComponent>(getId());
        timer.addEvent({2.0f, shoot_left_gun, 6});
        timer.addEvent({2.0f, shoot_right_gun, 6});
        timer.addEvent({1.0f, shoot_middle_gun, 12});
        timer.addEvent({12.f, [this](float t, int count)
                        { changeState(State::SpawningShips); }, 1});
    };
    // auto init_gun_shooting2 = [this, shoot_gun_at]()
    // {
    //     auto shoot_gun = [this, shoot_gun_at]()
    //     {
    //         auto target_pos = p_player->getPosition() + p_player->m_vel * randf(0, 0.016);
    //         shoot_gun_at(m_pos + utils::Vector2f{0., m_size.y * 0.4}, target_pos);
    //     };

    //     m_vel = {0, 0};
    //     auto &timer = m_world->m_systems.get<TimedEventComponent>(getId());
    //     timer.addEvent({0.5f, shoot_gun, 20});
    //     timer.addEvent({12.f, [this]()
    //                     { changeState(State::SpawningShips); }, 1});
    // };

    auto init_spawner = [this]()
    {
        deActivateShield();

        auto spawn_enemy = [this](float t, int count)
        {
            // auto &enemy = m_enemy_factory.create2(EnemyType::ShooterEnemy, m_pos + utils::Vector2f(-150.f, 0.f));
            // utils::Vector2f dr_to_target = p_player->getPosition() - enemy.getPosition();
            // enemy.m_vel = dr_to_target / utils::norm(dr_to_target) * enemy.m_max_vel;
            
        };
        auto spawn_laser_enemy = [this](float t, int count)
        {
            auto &enemy = m_enemy_factory.create2(EnemyType::LaserEnemyNoTarget, m_pos + utils::Vector2f(-55.f, 0.f));
            TargetComponent t_comp = {.p_target = nullptr, .target_pos = m_pos + utils::Vector2f(-300.f, 100.*(count - 2))};
            
            utils::Vector2f dr_to_target = t_comp.target_pos - enemy.getPosition();
            enemy.m_vel = dr_to_target / utils::norm(dr_to_target) * enemy.m_max_vel;
            TimedEventComponent timed_comp;
            t_comp.on_reaching_target =  [this, id = enemy.getId()]()
                                           {
                                               auto &t_comp = m_world->m_systems.get<TargetComponent>(id);
                                               t_comp.p_target = p_player;
                                               m_world->get(id)->m_max_vel *= 0.01f;
                                               m_world->m_systems.addEntity(id, LaserAIComponent{});
                                           };

            m_world->m_systems.addEntityDelayed(enemy.getId(), timed_comp, t_comp);
        };

        auto &timer = m_world->m_systems.get<TimedEventComponent>(getId());
        timer.addEvent({1.f, spawn_enemy, 3});
        timer.addEvent({1.f, spawn_laser_enemy, 5});
        timer.addEvent({15.f, [this](float t, int count)
                        { activateShield();
                            changeState(State::ShootingLasers); }, 1});
    };
    auto init_exposed = [this]()
    {
        auto &timer = m_world->m_systems.get<TimedEventComponent>(getId());
        timer.addEvent({20.f, [this](float t, int count)
                        { 
                            activateShield();changeState(State::ShootingLasers); }, 1});
    };

    m_on_state_change[State::ShootingLasers] = init_shooting_lasers;
    m_on_state_change[State::ShootingBigLaser] = init_big_laser;
    m_on_state_change[State::SpawningShips] = init_spawner;
    m_on_state_change[State::Exposed] = init_exposed;
    m_on_state_change[State::ShootingGuns] = init_gun_shooting;

    changeState(State::ShootingLasers);
}
void Boss1::onDestruction()
{
}
void Boss1::draw(LayersHolder &target)
{
    Sprite ship;
    ship.setPosition(m_pos);
    ship.setScale(m_size / 2.f);
    ship.setRotation(utils::to_radains * 90.f);
    ship.setTexture(*m_textures->get("Boss1"));

    if (m_state != State::SpawningShips)
    {
        auto &anim_comp = m_world->m_systems.get<AnimationComponent>(getId());

        Sprite shield;
        shield.setTexture(anim_comp.texture_id);
        shield.setPosition(m_pos);
        shield.setRotation(ship.getRotation());
        shield.setScale(m_size / 1.95);
        shield.m_tex_rect = anim_comp.tex_rect;
        shield.m_tex_size = anim_comp.texture_size;
        target.getCanvas("Shields").drawSprite(shield);
    }

    target.getCanvas("Unit").drawSprite(ship);
}
void Boss1::onCollisionWith(GameObject &obj, CollisionData &c_data)
{
}

Weapon::Weapon(GameWorld *world, TextureHolder &textures, PlayerEntity *player)
    : GameObject(world, textures, ObjectType::Boss)
{
}

void Weapon::update(float dt)
{
}
void Weapon::onCreation()
{
}
void Weapon::draw(LayersHolder &target)
{
}
void Weapon::onCollisionWith(GameObject &obj, CollisionData &c_data)
{
}

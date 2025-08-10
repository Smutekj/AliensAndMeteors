#include "Bosses.h"

#include "../GameWorld.h"
#include "../Utils/RandomTools.h"

Boss1::Boss1(GameWorld *world, TextureHolder &textures, Collisions::CollisionSystem *collider, PlayerEntity *player)
    : p_player(player), GameObject(world, textures, ObjectType::Boss)
{
    m_size = {398 / 2, 230 / 2};

    auto texture_size = static_cast<utils::Vector2i>(m_textures->get("LongShield")->getSize());

    m_shield_animation = std::make_unique<Animation>(texture_size,
                                                     2, 6, 0.5, 0, false);

    
}

void Boss1::update(float dt)
{

    m_shield_animation->update(dt);

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

void Boss1::onCreation()
{
    setAngle(90);

    Polygon hull = {4};
    hull.points = {{0.2, 0.5}, {0.2, -0.5}, {-0.2, -0.5}, {-0.2, 0.5}};
    hull.setScale(m_size / 2.);
    Polygon left_wing;
    left_wing.points = {{0.2, -0.3}, {0.5, 0.16}, {0.2, 0.14}};
    left_wing.setScale(m_size / 2.);
    Polygon right_wing;
    right_wing.points = {{-0.2, -0.3}, {-0.5, 0.16}, {-0.2, 0.14}};
    right_wing.setScale(m_size / 2.);
    CollisionComponent c_comp;
    c_comp.type = ObjectType::Boss;
    c_comp.shape.convex_shapes = {hull, left_wing, right_wing};

    HealthComponent health = {400, 400, 0};

    TimedEventComponent timer;
    
    AnimationComponent anim;
    anim.id = AnimationId::Shield;
    anim.cycle_duration = 0.69;
    
    m_world->m_systems.addEntity(getId(), timer, c_comp, health, anim);

    auto shoot_laser_at = [this](utils::Vector2f pos, utils::Vector2f target,
         utils::Vector2f offset, float width)
    {
        auto &laser = m_world->addObject2<Laser>();
        laser.setPosition(pos);
        laser.setAngle(utils::dir2angle(target - pos - offset));
        laser.setOwner(this);

        laser.m_rotates_with_owner = false;
        int direction = 2*(rand()%2) - 1;
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
    auto spawn_enemy_at = [this](utils::Vector2f pos)
    {
        auto player_pos = p_player->getPosition();
        auto &enemy = m_world->addObject2<Enemy>();
        enemy.setPosition(pos);
    };

    auto shoot_gun_at = [this](utils::Vector2f from, utils::Vector2f target)
    {
        auto& bullet = m_world->addObject2<Bullet>();
        bullet.setBulletType(BulletType::Fire);
        bullet.setTarget(p_player);
        bullet.setPosition(from);
        bullet.setSize(10);
        auto dr = target - from;
        bullet.m_vel = dr / utils::norm(dr) * 90.f;
    };
    
    auto init_shooting_lasers = [this, shoot_laser_at]()
    {
        auto shoot_laser_middle = [this, shoot_laser_at]()
        {
            auto target_pos = p_player->getPosition() ;
            shoot_laser_at(m_pos, target_pos, {0, 0}, 20.);
        };
        auto shoot_laser_upper = [this, shoot_laser_at]()
        {
            auto target_pos = p_player->getPosition() + p_player->m_vel * 0.2;
            shoot_laser_at(m_pos, target_pos, {0, m_size.y * 0.3}, 4.);
        };
        auto shoot_laser_lower = [this, shoot_laser_at]()
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
        timer.addEvent({10.0f, [this]()
                        { changeState(State::ShootingGuns); }, 1});
    };

    auto init_big_laser = [this, shoot_laser_at]()
    {
        auto shoot_laser_middle = [this, shoot_laser_at]()
        {
            auto target_pos = p_player->getPosition() + p_player->m_vel * 0.2;
            shoot_laser_at(m_pos, target_pos, {0, 0}, 20.);
        };
        
        m_vel = {-2, 0};
        auto &timer = m_world->m_systems.get<TimedEventComponent>(getId());
        timer.addEvent({3.f, shoot_laser_middle, 2});
        timer.addEvent({7.f, [this]()
                        { changeState(State::ShootingGuns); }, 1});
    };
    auto init_gun_shooting = [this, shoot_gun_at]()
    {
        auto shoot_gun = [this, shoot_gun_at]()
        {
            auto target_pos = p_player->getPosition() + p_player->m_vel * randf(0,0.016);
            shoot_gun_at(m_pos + utils::Vector2f{0., m_size.y * 0.4}, target_pos);
        };

        m_vel = {0, 0};
        auto &timer = m_world->m_systems.get<TimedEventComponent>(getId());
        timer.addEvent({0.5f, shoot_gun, 20});
        timer.addEvent({12.f, [this]()
                        { changeState(State::SpawningShips); }, 1});
    };

    auto init_spawner = [this, spawn_enemy_at]()
    {
        auto spawn_enemy = [this, spawn_enemy_at]()
        {
            spawn_enemy_at(m_pos + utils::Vector2f(-70.f, 0.f));
        };

        auto &timer = m_world->m_systems.get<TimedEventComponent>(getId());
        timer.addEvent({1.f, spawn_enemy, 7});
        timer.addEvent({8.f, [this]()
                        { changeState(State::Exposed); }, 1});
    };
    auto init_exposed = [this]()
    {
        auto &timer = m_world->m_systems.get<TimedEventComponent>(getId());
        timer.addEvent({20.f, [this]()
                        { changeState(State::ShootingLasers); }, 1});
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

    if (m_state != State::Exposed)
    {
        auto& anim_comp = m_world->m_systems.get<AnimationComponent>(getId());

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

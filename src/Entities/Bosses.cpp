#include "Bosses.h"

#include "../GameWorld.h"
#include "../Utils/RandomTools.h"

Boss1::Boss1(GameWorld *world, TextureHolder &textures, Collisions::CollisionSystem *collider, PlayerEntity *player)
    : p_player(player), GameObject(world, textures, ObjectType::Boss)
{
    // m_collision_shape = std::make_unique<Polygon>(4);
    m_size = {230 / 2, 398 / 2};
    // m_collision_shape->setScale(m_size / 2.);

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
    Polygon hull = {4};
    hull.points = {{0.7, 0.2}, {-0.7, 0.2}, {-0.7, -0.2}, {0.7, -0.2}};
    hull.setScale(m_size / 2.);
    Polygon left_wing;
    left_wing.points = {{0.3, 0.2}, {-0.3, 1.}, {-0.3, 0.2}};
    left_wing.setScale(m_size / 2.);
    Polygon right_wing;
    right_wing.points = {{0.3, -0.2}, {-0.3, -1.}, {-0.3, -0.2}};
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

    auto shoot_laser_at = [this](utils::Vector2f pos, utils::Vector2f offset, float width)
    {
        auto &laser = m_world->addObject2<Laser>();
        laser.setPosition(pos);
        laser.setAngle(utils::dir2angle(Vec2{-1.f, randf(-0.2f, 0.2f)}));
        laser.setOwner(this);

        laser.m_rotates_with_owner = false;
        laser.m_updater = [&laser](float dt)
        {
            laser.setAngle(laser.getAngle() + 5. * dt);
        };
        laser.m_offset = offset;
        laser.m_max_width = width;
        laser.m_max_length = 500.;
        laser.m_life_time = 3.;
        laser.m_max_dmg = 0.1f;
    };
    auto spawn_enemy_at = [this](utils::Vector2f pos)
    {
        auto player_pos = p_player->getPosition();
        auto &enemy = m_world->addObject2<Enemy>();
        enemy.setPosition(pos);
    };

    auto init_shooting_lasers = [this, shoot_laser_at]()
    {
        auto shoot_laser_upper = [this, shoot_laser_at]()
        {
            shoot_laser_at(m_pos, {0, m_size.y * 0.3}, 4.);
        };
        auto shoot_laser_lower = [this, shoot_laser_at]()
        {
            shoot_laser_at(m_pos, {0, -m_size.y * 0.3}, 4.f);
        };
        m_vel = {-1, 0};

        auto &timer = m_world->m_systems.get<TimedEventComponent>(getId());
        timer.addEvent({5.f, shoot_laser_lower, 2});
        timer.addEvent({2.5f, shoot_laser_upper, 4});
        timer.addEvent({10.1f, [this]()
                        { changeState(State::ShootingBigLaser); }, 1});
    };

    auto init_big_laser = [this, shoot_laser_at]()
    {
        auto shoot_laser_middle = [this, shoot_laser_at]()
        {
            shoot_laser_at(m_pos, {0, 0}, 20.);
        };

        m_vel = {-1, 0};
        auto &timer = m_world->m_systems.get<TimedEventComponent>(getId());
        timer.addEvent({5.f, shoot_laser_middle, 2});
        timer.addEvent({15.f, [this]()
                        { changeState(State::SpawningShips); }, 1});
    };

    auto init_spawner = [this, spawn_enemy_at]()
    {
        auto spawn_enemy = [this, spawn_enemy_at]()
        {
            spawn_enemy_at(m_pos + utils::Vector2f(-20.f, 0.f));
        };

        auto &timer = m_world->m_systems.get<TimedEventComponent>(getId());
        timer.addEvent({1.f, spawn_enemy, 10});
        timer.addEvent({20.f, [this]()
                        { changeState(State::ShootingLasers); }, 1});
    };

    m_on_state_change[State::ShootingLasers] = init_shooting_lasers;
    m_on_state_change[State::ShootingBigLaser] = init_big_laser;
    m_on_state_change[State::SpawningShips] = init_spawner;

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
        shield.setScale(m_size / 1.95);
        shield.m_tex_rect = anim_comp.tex_rect;
        shield.m_tex_size = anim_comp.texture_size;
        target.getCanvas("Unit").drawSprite(shield);
        
    }

    target.getCanvas("Unit").drawSprite(ship);
}
void Boss1::onCollisionWith(GameObject &obj, CollisionData &c_data)
{
    if (obj.getType() == ObjectType::Laser)
    {
        m_world->p_messenger->send(DamageReceivedEvent{ObjectType::Laser, obj.getId(), ObjectType::Bomb, m_id, 0.1});
    }
}

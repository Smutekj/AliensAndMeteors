#include "Bosses.h"

#include "../GameWorld.h"
#include "../Utils/RandomTools.h"

Boss1::Boss1(GameWorld *world, TextureHolder &textures, Collisions::CollisionSystem *collider, PlayerEntity *player)
    : p_player(player), GameObject(world, textures, ObjectType::Boss)
{
    m_collision_shape = std::make_unique<Polygon>(4);
    m_size = {230 / 2, 398 / 2};
    m_collision_shape->setScale(m_size / 2.);
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
    TimedEventComponent timer;
    m_world->m_systems.add(timer, getId());

    auto shoot_laser_at = [this](utils::Vector2f pos, utils::Vector2f offset, float width)
    {
        auto &laser = m_world->addObject2<Laser>();
        laser.setPosition(pos);
        setAngle(utils::dir2angle(Vec2{-1.f, randf(-0.2f, 0.2f)}));
        laser.setOwner(this);
        
        laser.m_offset = offset;
        laser.m_max_width = width;
        laser.m_max_length = 500.;
        laser.m_life_time = 3.;
        laser.m_max_dmg = 0.1f;
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
            m_state = State::ShootingBigLaser;
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
        timer.addEvent({15.f, [this](){changeState(State::ShootingLasers);}, 1});
    };

    m_on_state_change[State::ShootingLasers] = init_shooting_lasers;
    m_on_state_change[State::ShootingBigLaser] = init_big_laser;

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

    target.getCanvas("Unit").drawSprite(ship);
}
void Boss1::onCollisionWith(GameObject &obj, CollisionData &c_data)
{
}

#include "Bosses.h"

#include "../GameWorld.h"

Boss1::Boss1(GameWorld *world, TextureHolder &textures, Collisions::CollisionSystem *collider, PlayerEntity *player)
: p_player(player), GameObject(world, textures, ObjectType::Boss)
{
    m_collision_shape = std::make_unique<Polygon>(4);
    m_size = {230/2, 398/2};
    m_collision_shape->setScale(m_size / 2.);
}


void Boss1::update(float dt)
{
    //! periodic up-down motion
    m_motion_time += dt;
    float motion_frequency = 2.f * 3.14f / m_motion_period;
    float motion_amplitude = (m_size.y * 0.2f);
    m_pos.y += dt * motion_amplitude * motion_frequency * std::cos(m_motion_time * motion_frequency);


    if (m_state == State::ShootingLasers)
    {
        auto shoot_laser_at = [this](utils::Vector2f pos, utils::Vector2f offset)
        {
            auto &laser = m_world->addObject2<Laser>();
            laser.setPosition(pos);
            setAngle(utils::dir2angle(Vec2{-1.f, 0.f}));
            laser.setOwner(this);
            laser.m_offset = offset;
            laser.m_max_width = 4.;
            laser.m_max_length = 500.;
            laser.m_life_time = 3.;
        };
        auto shoot_laser_upper = [this, shoot_laser_at]()
        {
            shoot_laser_at(m_pos, {0, m_size.y * 0.3});
        };
        auto shoot_laser_lower = [this, shoot_laser_at]()
        {
            shoot_laser_at(m_pos, {0, - m_size.y * 0.3});
        };
        m_vel = {-1, 0};
        TimedEventComponent timer;
        timer.addEvent({10.f, shoot_laser_lower, 1});
        timer.addEvent({5.f, shoot_laser_upper, 1});
        m_world->m_systems.add(timer, getId());
        
        m_state = State::ShootingBigLaser;
    }
}
void Boss1::onCreation()
{
}
void Boss1::onDestruction()
{
}
void Boss1::draw(LayersHolder &target)
{
    Sprite ship;
    ship.setPosition(m_pos);
    ship.setScale(m_size/2.f);
    ship.setRotation(utils::to_radains * 90.f);
    ship.setTexture(*m_textures->get("Boss1"));

    target.getCanvas("Unit").drawSprite(ship);
}
void Boss1::onCollisionWith(GameObject &obj, CollisionData &c_data)
{
}

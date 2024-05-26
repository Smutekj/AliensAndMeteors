#include "Player.h"
#include "ResourceHolder.h"
// #include "Particles.h"

PlayerEntity::PlayerEntity(GameWorld *world, TextureHolder &textures)
    : GameObject(world, textures, ObjectType::Player)
{
    m_collision_shape = std::make_unique<Polygon>(4);
    m_collision_shape->setScale({2 * m_radius, 2 * m_radius});

    m_particles_left = std::make_unique<Particles>(m_pos, 100);
    m_particles_right = std::make_unique<Particles>(m_pos, 100);
}

void PlayerEntity::update(float dt)
{
    fixAngle();
    boost();

    speed += acceleration;
    float real_max_speed = max_speed * (1 + boost_factor * is_boosting);
    speed += boost_factor * is_boosting;
    speed -= speed * slowing_factor;

    m_vel = speed * angle2dir(m_angle);
    m_pos += m_vel * dt;

    m_particles_left->update(dt);
    m_particles_left->setSpawnPos(m_pos - m_radius * angle2dir(m_angle + 40));
    m_particles_right->update(dt);
    m_particles_right->setSpawnPos(m_pos - m_radius * angle2dir(m_angle - 40));
}

void PlayerEntity::onCollisionWith(GameObject &obj, CollisionData &c_data)
{
    switch (obj.getType())
    {
    case ObjectType::Meteor:
    {
        auto mvt = c_data.separation_axis;
        if (dot(mvt, m_vel) < 0.f)
        {
            m_vel -= 2.f * dot(mvt, m_vel) * mvt;
            m_angle = dir2angle(m_vel);
            health -= 1;
        }
        break;
    }
    case ObjectType::Bullet:
    {
        // health--;
        break;
    }
    case ObjectType::Heart:
    {
        health += 5;
        break;
    }
    }
}

void PlayerEntity::draw(sf::RenderTarget &target)
{

    m_player_shape.setOrigin({m_radius, m_radius});
    m_player_shape.setPosition(m_pos);
    m_player_shape.setRotation(m_angle);
    m_player_shape.setSize({2 * m_radius, 2 * m_radius});
    m_player_shape.setTexture(&m_textures.get(Textures::ID::PlayerShip));

    target.draw(m_player_shape);
    m_particles_left->draw(target);
    m_particles_right->draw(target);
}
void PlayerEntity::onCreation() {}
void PlayerEntity::onDestruction() {}

void PlayerEntity::fixAngle()
{
    if (m_angle < -180.f)
    {
        m_angle = 180.f;
    }
    else if (m_angle > 180.f)
    {
        m_angle = -180.f;
    }
}

void PlayerEntity::boost()
{
    if (is_boosting)
    {
        boost_time++;
        boost_heat++;

        if (boost_heat > max_boost_heat)
        {
            is_boosting = false;
            boost_time = 0;
        }
    }
    else
    {
        if (boost_heat >= 0.f)
        {
            boost_heat--;
        }
    }
}
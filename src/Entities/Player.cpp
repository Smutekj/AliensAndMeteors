#include "Player.h"

#include <SFML/Graphics/RenderTarget.hpp>

#include "../ResourceHolder.h"
#include "Entities.h"
#include "Attacks.h"

PlayerEntity::PlayerEntity(GameWorld *world, TextureHolder &textures)
    : GameObject(world, textures, ObjectType::Player)
{
    m_collision_shape = std::make_unique<Polygon>(4);
    m_collision_shape->setScale({2 * m_radius, 2 * m_radius});

    m_particles_left = std::make_unique<Particles>(100);
    m_particles_right = std::make_unique<Particles>( 100);
}

void PlayerEntity::update(float dt) 
{
    fixAngle();
    boost();

    speed += acceleration;
    speed += boost_factor * is_boosting;
    speed -= speed * slowing_factor;

    m_vel = speed * angle2dir(m_angle);

    if (m_deactivated_time > 0)
    {
        m_deactivated_time -= dt;
        m_vel /= 2.f;
        is_boosting = false;
    }
    m_pos += m_vel * dt;

    m_particles_left->setSpawnPos(m_pos - m_radius * angle2dir(m_angle + 40));
    m_particles_left->update(dt);
    m_particles_right->setSpawnPos(m_pos - m_radius * angle2dir(m_angle - 40));
    m_particles_right->update(dt);
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
    case ObjectType::Explosion:
    {
        health -= 0.1f;
        break;
    }
    case ObjectType::Bullet:
    {
        health--;
        break;
    }
    case ObjectType::Heart:
    {
        health += 5;
        break;
    }
    case ObjectType::Laser:
    {
        if (static_cast<Laser &>(obj).getOwner() != this)
        {
            health -= 0.1f;
        }
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

    if (is_boosting)
    {
        m_particles_left->setColor({255, 128, 0});
        m_particles_right->setColor({255, 128, 0});
        m_particles_left->setVel(5);
        m_particles_right->setVel(5);
    }
    else
    {
        m_particles_left->setColor({255, 255, 255});
        m_particles_right->setColor({255, 255, 255});
        m_particles_left->setVel(3);
        m_particles_right->setVel(3);
    }

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
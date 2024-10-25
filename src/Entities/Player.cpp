#include "Player.h"

#include "../DrawLayer.h"
#include "../Utils/RandomTools.h"
// #include "../ResourceHolder.h"
#include "Entities.h"
#include "Attacks.h"

PlayerEntity::PlayerEntity(GameWorld *world, TextureHolder &textures)
    : GameObject(world, textures, ObjectType::Player)
{
    m_collision_shape = std::make_unique<Polygon>(4);
    m_collision_shape->setScale(2 * m_radius, 2 * m_radius);

    m_particles_left = std::make_unique<Particles>(100);
    m_particles_right = std::make_unique<Particles>(100);

    auto basic_emitter = [this](utils::Vector2f spawn_pos)
    {
        Particle new_part;
        new_part.pos = spawn_pos;
        new_part.color = {1, 0, 1, 1};
        new_part.vel = 0. * m_vel - 0.8 * utils::angle2dir(m_angle - 180 + randf(-60, 60));
        return new_part;
    };

    auto basic_updater = [](Particle &part, float dt)
    {
        part.pos = part.pos + part.vel*dt;
        part.scale += utils::Vector2f{0.075, 0.075};
    };

    m_particles_left->setEmitter(basic_emitter);
    m_particles_right->setEmitter(basic_emitter);
    m_particles_left->setUpdater(basic_updater);
    m_particles_right->setUpdater(basic_updater);

    m_particles_left->setFinalColor({0,0,0,0});
    m_particles_right->setFinalColor({0,0,0,0});
}

void PlayerEntity::update(float dt)
{
    fixAngle();
    boost();

    speed += acceleration*dt;
    speed += boost_factor*dt * is_boosting;
    speed -= speed * slowing_factor;

    m_vel = speed * utils::angle2dir(m_angle);

    if (m_deactivated_time > 0)
    {
        m_deactivated_time -= dt;
        m_vel /= 2.f;
        is_boosting = false;
    }
    m_pos += m_vel * dt;

    m_particles_left->setSpawnPos(m_pos - m_radius * utils::angle2dir(m_angle + 40));
    m_particles_left->update(dt);
    m_particles_right->setSpawnPos(m_pos - m_radius * utils::angle2dir(m_angle - 40));
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
            m_angle = utils::dir2angle(m_vel);
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

void PlayerEntity::draw(LayersHolder &layers)
{

    auto &target = layers.getCanvas("Unit");
    auto &shiny_target = layers.getCanvas("Bloom");

    m_player_shape.setPosition(m_pos);
    m_player_shape.setRotation(glm::radians(m_angle));
    m_player_shape.setScale(2 * m_radius, 2 * m_radius);
    m_player_shape.setTexture(*m_textures.get("PlayerShip"));

    target.drawSprite(m_player_shape, "Instanced");

    if (is_boosting)
    {
        m_particles_left->setInitColor({5., 2., 0, 0.2});
        m_particles_right->setInitColor({5., 2., 0, 0.2});
    }
    else
    {
        m_particles_left->setInitColor({25, 3, 0, 0.05});
        m_particles_right->setInitColor({25, 3, 0, 0.05});
    }

    m_particles_left->draw(shiny_target);
    m_particles_right->draw(shiny_target);
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
        if (boost_heat > 0.f)
        {
            boost_heat--;
        }
    }
}
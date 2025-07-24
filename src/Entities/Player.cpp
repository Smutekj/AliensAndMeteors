#include "Player.h"

#include "../DrawLayer.h"
#include "../Utils/RandomTools.h"
// #include "../ResourceHolder.h"
#include "Entities.h"
#include "Attacks.h"

PlayerEntity::PlayerEntity(GameWorld *world, TextureHolder &textures, Collisions::CollisionSystem *collider, PlayerEntity *player)
    : GameObject(world, textures, ObjectType::Player, collider, player)
{
    m_collision_shape = std::make_unique<Polygon>(4);
    m_collision_shape->setScale(2 * m_radius, 2 * m_radius);
}

void PlayerEntity::update(float dt)
{
    fixAngle();
    boost(dt);
    if (m_is_turning_left)
    {
        //! slower turning when shooting laser
        auto angle_vel = m_angle_vel - m_angle_vel * 0.4 * m_is_shooting_laser;
        setAngle(m_angle + angle_vel * dt);
    }
    if (m_is_turning_right)
    {
        auto angle_vel = m_angle_vel - m_angle_vel * 0.4 * m_is_shooting_laser;
        setAngle(m_angle - angle_vel * dt);
    }
    if (m_is_shooting_laser)
    {
        m_laser_timer -= dt;
        if (m_laser_timer <= 0.)
        {
            m_laser_timer = 0.;
            m_is_shooting_laser = false;
        }
    }

    bool is_boosting = booster == BoosterState::Boosting;
    auto acc = acceleration + 2 * acceleration * is_boosting;
    speed += acc * dt;
    m_vel = (speed)*utils::angle2dir(m_angle);
    // utils::truncate(m_vel, max_speed + is_boosting * max_speed);
    if (m_deactivated_time > 0)
    {
        m_deactivated_time -= dt;
        m_vel /= 2.f;
        booster = BoosterState::Ready;
    }
    m_pos += m_vel * dt;
    //! speed fallout
    if (speed > boost_max_speed)
    {
        speed -= speed * 1.1 * slowing_factor * dt;
    }
    else
    {
        // speed -=  10. * slowing_factor * dt;
        // speed = std::max(0.f, speed);
    }

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
        health -= 0.5f;
        break;
    }
    case ObjectType::Bullet:
    {
        health--;
        break;
    }
    case ObjectType::Heart:
    {
        health += 3;
        break;
    }
    }
}

void PlayerEntity::draw(LayersHolder &layers)
{

    auto &target = layers.getCanvas("Unit");
    auto &shiny_target = layers.getCanvas("Bloom");

    // shiny_target.drawSprite(m_player_shape, "boostBar");

    m_player_shape.setPosition(m_pos);
    m_player_shape.setScale(2 * m_radius, 2 * m_radius);
    m_player_shape.setRotation(glm::radians(m_angle));
    m_player_shape.setTexture(*m_textures->get("PlayerShip"));
    target.drawSprite(m_player_shape);

    if (booster == BoosterState::Boosting)
    {
        m_particles_left->setInitColor({50., 1, 0, 1.0});
        m_particles_right->setInitColor({50., 1, 0, 1.0});
    }
    else
    {
        m_particles_left->setInitColor({1., 1., 0, 1.0});
        m_particles_right->setInitColor({1., 1., 0, 1.0});
    }

    m_particles_left->draw(shiny_target);
    m_particles_right->draw(shiny_target);
}
void PlayerEntity::onCreation()
{
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
        part.pos = part.pos + part.vel * dt;
        part.scale += utils::Vector2f{0.075, 0.075}*dt;
    };

    m_particles_left->setEmitter(basic_emitter);
    m_particles_right->setEmitter(basic_emitter);
    m_particles_left->setUpdater(basic_updater);
    m_particles_right->setUpdater(basic_updater);

    m_particles_left->setFinalColor({1, 0, 0, 0.1});
    m_particles_right->setFinalColor({1, 0, 0, 0.1});
}
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

void PlayerEntity::boost(float dt)
{
    if (booster == BoosterState::Boosting && m_fuel > 0)
    {
        boost_heat += 0.01 * dt;
        m_fuel -= 0.1 * dt;

        if (boost_heat > max_boost_heat)
        {
            booster = BoosterState::Disabled;
        }
    }
    if (booster != BoosterState::Boosting)
    {
        boost_heat -= 0.075 * dt;
    }

    if (m_fuel < 0)
    {
        m_fuel = 0.;
        booster = BoosterState::Disabled;
    }

    if (boost_heat < 0.)
    {
        boost_heat = 0.;
        booster = BoosterState::Ready;
    }
}
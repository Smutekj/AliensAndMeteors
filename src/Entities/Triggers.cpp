#include "Triggers.h"

#include <SFML/Graphics/RenderTarget.hpp>

#include "Player.h"

Trigger::Trigger(GameWorld *world, TextureHolder &textures)
    : GameObject(world, textures, ObjectType::Trigger)
{
}

Timer::Timer(GameWorld *world, TextureHolder &textures)
    : Trigger(world, textures) {}

void Timer::onCreation() {}
void Timer::onDestruction() {}
void Timer::draw(sf::RenderTarget &target) {}
void Timer::onCollisionWith(GameObject &obj, CollisionData &c_data) {}

void Timer::update(float dt)
{
    m_time += dt;
    if (m_time > m_cooldown)
    {
        m_time = 0;
        m_callback();
    }
}

ReachPlace::ReachPlace(GameWorld *world, TextureHolder &textures, PlayerEntity *player)
    : m_player(player), Trigger(world, textures)
{
    m_collision_shape = std::make_unique<Polygon>(10);
    m_collision_shape->setScale(4, 4);
    m_callback = []() {};
}
ReachPlace::~ReachPlace() {}

void ReachPlace::onCreation()
{
}
void ReachPlace::onDestruction()
{
}
void ReachPlace::update(float dt) {}

void ReachPlace::draw(sf::RenderTarget &target)
{
    sf::CircleShape m_spot;
    m_spot.setRadius(10);
    m_spot.setFillColor(sf::Color(125, 125, 0, 125));
    m_spot.setOrigin({10, 10});
    m_spot.setPosition(m_pos);

    target.draw(m_spot);
}

void ReachPlace::onCollisionWith(GameObject &obj, CollisionData &c_data)
{
    if (obj.getType() == ObjectType::Player)
    {
        callback();
    }
}

EntityDestroyed::EntityDestroyed(GameWorld *world, TextureHolder &textures, GameObject *entity)
    : m_watched_entity(entity), Trigger(world, textures)
{
}
EntityDestroyed::~EntityDestroyed() {}

void EntityDestroyed::onCreation()
{
}
void EntityDestroyed::onDestruction()
{
}
void EntityDestroyed::update(float dt)
{
    if (m_watched_entity && m_watched_entity->isDead())
    {
        kill();
        callback();
    }
}

void EntityDestroyed::draw(sf::RenderTarget &target)
{
    sf::CircleShape m_spot;
    m_spot.setRadius(10);
    m_spot.setFillColor(sf::Color(255, 0, 0, 125));
    m_spot.setOrigin({10, 10});
    m_spot.setPosition(m_watched_entity->getPosition());

    target.draw(m_spot);
}

void EntityDestroyed::onCollisionWith(GameObject &obj, CollisionData &c_data)
{
}

StayAtPlace::StayAtPlace(GameWorld *world, TextureHolder &textures, PlayerEntity *player, float trigger_time)
    : m_trigger_time(trigger_time), m_player(player), Trigger(world, textures)
{
}

void StayAtPlace::update(float dt)
{
    if (dist(m_player->getPosition(), m_pos) < 10.f)
    {
        m_stay_time += dt;
        if (m_stay_time > m_trigger_time)
        {
            callback();
        }
    }else
    {
        m_stay_time = 0.f;
    }
}
void StayAtPlace::onCreation() {}
void StayAtPlace::onDestruction() {}
void StayAtPlace::draw(sf::RenderTarget &target)
{
    sf::CircleShape m_spot;
    m_spot.setRadius(10);
    m_spot.setFillColor(sf::Color(125, 125, 0, 125));
    m_spot.setOrigin({10, 10});
    m_spot.setPosition(m_pos);

    target.draw(m_spot);
}
void StayAtPlace::onCollisionWith(GameObject &obj, CollisionData &c_data)
{
}

#include "Triggers.h"
#include "Player.h"
#include "../DrawLayer.h"
#include "../GameWorld.h"

Trigger::Trigger(GameWorld *world, TextureHolder &textures)
    : GameObject(world, textures, ObjectType::Trigger)
{
}

Timer::Timer(GameWorld *world, TextureHolder &textures)
    : Trigger(world, textures) {}

void Timer::onCreation() {}
void Timer::onDestruction() {}
void Timer::draw(LayersHolder &target) {}
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

}
ReachPlace::~ReachPlace() {}

void ReachPlace::onCreation()
{
    CollisionComponent c_comp;
    Polygon shape= {10};
    c_comp.shape.convex_shapes.push_back(shape);
    c_comp.type = ObjectType::Trigger;
    m_world->m_systems.add(c_comp, getId());
}

void ReachPlace::onDestruction()
{
}
void ReachPlace::update(float dt) {}

void ReachPlace::draw(LayersHolder &layers)
{
    if(m_active)
    {
        auto& target = layers.getCanvas("Unit");
        target.drawCricleBatched(m_pos, 10.f, {0.5, 0.5, 0, 0.5});
    }
}

void ReachPlace::onCollisionWith(GameObject &obj, CollisionData &c_data)
{
    if (obj.getType() == ObjectType::Player && m_active)
    {
        callback();
    }
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
void StayAtPlace::draw(LayersHolder &layers)
{
    auto& target = layers.getCanvas("Unit");
    target.drawCricleBatched(m_pos, 10.f, {0,1,0,0.4});
}
void StayAtPlace::onCollisionWith(GameObject &obj, CollisionData &c_data)
{
}

#include "Entities.h"

#include "../DrawLayer.h"
#include "../CollisionSystem.h"
#include "../BehaviourBase.h"
#include "../GameWorld.h"
#include "../Animation.h"
#include "../Polygon.h"
#include "../Utils/RandomTools.h"

#include "Enemy.h"
#include "Player.h"
#include "Attacks.h"


Explosion::Explosion(GameWorld *world, TextureHolder &textures, Collisions::CollisionSystem *collider,  PlayerEntity *player)
    : GameObject(world, textures, ObjectType::Explosion)
{
    m_collision_shape = std::make_unique<Polygon>(4);

    setType("Explosion");
}

Explosion::~Explosion() {}

void Explosion::setType(std::string texture_id)
{
    //! TODO: ADD some animation manager
    if (texture_id == "Explosion2")
    {
        m_explosion_sprite.setTexture(*m_textures->get(texture_id));
        auto texture_size = static_cast<utils::Vector2i>(m_textures->get(texture_id)->getSize());
        m_animation = std::make_unique<Animation>(texture_size,
                                                  12, 1, m_life_time, 1, true);
    }
    else if (texture_id == "Explosion")
    {
        m_explosion_sprite.setTexture(*m_textures->get(texture_id));
        auto texture_size = static_cast<utils::Vector2i>(m_textures->get(texture_id)->getSize());
        m_life_time = 0.5f;
        m_animation = std::make_unique<Animation>(texture_size,
                                                  4, 4, m_life_time , 1, false);
    }
}

void Explosion::update(float dt)
{
    m_time += dt;
    if (m_time > m_life_time)
    {
        kill();
    }
    if(m_is_expanding)
    {
        m_explosion_radius = m_time / m_life_time * m_max_explosion_radius;
    }else{
        m_explosion_radius = m_max_explosion_radius;
    }
    setSize({m_explosion_radius, m_explosion_radius});
    // if (m_collision_shape)
    // {
    //     m_collision_shape->setScale(2 * m_explosion_radius, 2 * m_explosion_radius);
    // }
    m_animation->update(dt);

    m_pos += m_vel * dt;
}

void Explosion::onCollisionWith(GameObject &obj, CollisionData &c_data)
{
}

void Explosion::onCreation()
{
}

void Explosion::onDestruction()
{
}

void Explosion::draw(LayersHolder& layers)
{
    auto& target = layers.getCanvas("Unit");
    m_explosion_sprite.m_tex_rect = m_animation->getCurrentTextureRect();
    m_explosion_sprite.setPosition(m_pos);
    m_explosion_sprite.setRotation(m_angle);
    m_explosion_sprite.setScale(2 * m_explosion_radius, 2 * m_explosion_radius);
    target.drawSprite(m_explosion_sprite);
}

EMP::EMP(GameWorld *world, TextureHolder &textures, Collisions::CollisionSystem *collider,  PlayerEntity *player)
    : m_collider(collider), GameObject(world, textures, ObjectType::EMP)
{
    m_collision_shape = std::make_unique<Polygon>(8);
    m_collision_shape->setScale(2, 2);
    auto texture_size = static_cast<utils::Vector2i>(m_textures->get("Bomb")->getSize());

    m_animation = std::make_unique<Animation>(texture_size,
                                              7, 2, m_life_time, 0);

    m_texture_rect.setTexture(*m_textures->get("Bomb"));
    m_texture_rect.setScale(2, 2);
}

EMP::~EMP() {}

void EMP::update(float dt)
{
    if (m_is_ticking)
    {
        m_vel -= 0.05f * m_vel;

        m_time += dt;
        if (m_time > m_life_time)
        {
            m_time = 0;
            m_is_ticking = false;
            m_collision_shape = nullptr;
            m_collider->removeObject(*this);

            auto texture_size = static_cast<utils::Vector2i>(m_textures->get("Emp")->getSize());
            m_texture_rect.setTexture(*m_textures->get("Emp"));
            m_animation = std::make_unique<Animation>(texture_size,
                                                      8, 1, m_life_time, 0, true);

            onExplosion();
        }
    }

    if (!m_is_ticking)
    {
        m_time += dt;
        if (m_time > m_life_time)
        {
            kill();
        }

        // m_explosion_radius = m_time / m_life_time * m_max_explosion_radius;
        // m_collision_shape->setScale({2 * m_explosion_radius, 2 * m_explosion_radius});
    }
    m_animation->update(dt);

    m_pos += m_vel * dt;
}

void EMP::onCollisionWith(GameObject &obj, CollisionData &c_data)
{
}

void EMP::onCreation()
{
}

void EMP::onDestruction()
{
}

void EMP::onExplosion()
{
    auto enemies = m_collider->findNearestObjects(ObjectType::Enemy, m_pos, m_explosion_radius);
    for (auto enemy : enemies)
    {
        static_cast<Enemy *>(enemy)->m_deactivated = true;
        static_cast<Enemy *>(enemy)->m_deactivated_time = 2.0f;
    }
    auto players = m_collider->findNearestObjects(ObjectType::Player, m_pos, m_explosion_radius);
    for (auto player : players)
    {
        static_cast<PlayerEntity *>(player)->m_deactivated_time = 2.0f;
    }
}

void EMP::draw(LayersHolder& layers)
{

    auto& target = layers.getCanvas("Unit");

    m_texture_rect.m_tex_rect = m_animation->getCurrentTextureRect();

    if (!m_is_ticking)
    {

        m_texture_rect.setScale(2 * m_explosion_radius, 2 * m_explosion_radius);
    }
    m_texture_rect.setPosition(m_pos);
    m_texture_rect.setRotation(m_angle);
    target.drawSprite(m_texture_rect);
}

ExplosionAnimation::ExplosionAnimation(GameWorld *world, TextureHolder &textures, Collisions::CollisionSystem *collider,  PlayerEntity *player)
    : GameObject(world, textures, ObjectType::Explosion)
{
    m_collision_shape = std::make_unique<Polygon>(4);
    m_collision_shape->setScale(2 * m_explosion_radius, 2 * m_explosion_radius);

    auto texture_size = static_cast<utils::Vector2i>(m_textures->get("Explosion")->getSize());

    m_animation = std::make_unique<Animation>(texture_size,
                                              4, 4, m_life_time, 1, false);
}

ExplosionAnimation::~ExplosionAnimation() {}

void ExplosionAnimation::update(float dt)
{
    m_life_time -= dt;

    if (m_life_time < 0.f)
    {
        kill();
    }

    m_animation->update(dt);

    m_pos += m_vel * dt;
}

void ExplosionAnimation::onCollisionWith(GameObject &obj, CollisionData &c_data)
{
}

void ExplosionAnimation::onCreation()
{
}

void ExplosionAnimation::onDestruction()
{
}

void ExplosionAnimation::draw(LayersHolder& layers)
{
    auto& target = layers.getCanvas("Unit");

    Sprite rect;
    rect.setTexture(*m_textures->get("Explosion"));
    rect.m_tex_rect = m_animation->getCurrentTextureRect();
    rect.m_color = {255, 255, 255, 155};

    rect.setPosition(m_pos);
    rect.setRotation(m_angle);
    rect.setScale(2 * m_explosion_radius, 2 * m_explosion_radius);
    target.drawSprite(rect);
}

Heart::Heart(GameWorld *world, TextureHolder &textures, Collisions::CollisionSystem *collider,  PlayerEntity *player, Pickup type)
    : GameObject(world, textures, ObjectType::Heart)
{
    m_collision_shape = std::make_unique<Polygon>(4);
    m_collision_shape->setScale(3, 3);
    
    // m_rigid_body = std::make_unique<RigidBody>();
    // m_rigid_body->mass = 0.01f;
}

Heart::~Heart() {}

void Heart::update(float dt)
{
    m_pos += m_vel * dt;
}
void Heart::onCreation()
{
}

void Heart::onDestruction()
{
    auto& effect = m_world->addVisualEffect(EffectType::ParticleEmiter);
    effect.setPosition(m_pos);
    effect.setLifeTime(2.f);
}

void Heart::onCollisionWith(GameObject &obj, CollisionData &c_data)
{
    if (obj.getType() == ObjectType::Player)
    {
        kill();
    }
}

void Heart::draw(LayersHolder& layers)
{
    auto& target = layers.getCanvas("Unit");

    Sprite rect;
    if(type==Pickup::Heart)
    {
        rect.setTexture(*m_textures->get("Heart"));
    }else if(type == Pickup::Fuel)
    {
        rect.setTexture(*m_textures->get("Fuel"));
    }
    rect.setPosition(m_pos);
    rect.setScale(3., 3.);
    target.drawSprite(rect);
}

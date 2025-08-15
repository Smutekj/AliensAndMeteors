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

Explosion::Explosion(GameWorld *world, TextureHolder &textures, PlayerEntity *player)
    : GameObject(world, textures, ObjectType::Explosion)
{
}

Explosion::~Explosion() {}

void Explosion::setType(AnimationId id)
{
    if (!m_world->m_systems.has<AnimationComponent>(getId()))
    {
        AnimationComponent anim;
        anim.id = id;
        anim.cycle_duration = 2.;
        m_world->m_systems.add<AnimationComponent>(anim, getId());
    }
    else
    {
        m_world->m_systems.get<AnimationComponent>(getId()).id = id;
    }
}

void Explosion::update(float dt)
{
    m_time += dt;
    if (m_time > m_life_time)
    {
        kill();
    }
    if (m_is_expanding)
    {
        m_explosion_radius = m_time / m_life_time * m_max_explosion_radius;
    }
    else
    {
        m_explosion_radius = m_max_explosion_radius;
    }
    setSize({m_explosion_radius, m_explosion_radius});

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

void Explosion::draw(LayersHolder &layers)
{
    auto &target = layers.getCanvas("Unit");

    // assert(m_world->m_systems.has<AnimationComponent>(getId()));
    if (m_world->m_systems.has<AnimationComponent>(getId()))
    {
        auto &anim = m_world->m_systems.get<AnimationComponent>(getId());

        m_explosion_sprite.m_tex_rect = anim.tex_rect;
        m_explosion_sprite.m_tex_size = anim.texture_size;
        m_explosion_sprite.setTexture(anim.texture_id, 0);
        m_explosion_sprite.setPosition(m_pos);
        m_explosion_sprite.setRotation(m_angle);
        m_explosion_sprite.setScale(2 * m_explosion_radius, 2 * m_explosion_radius);
        target.drawSprite(m_explosion_sprite);
    }
}

EMP::EMP(GameWorld *world, TextureHolder &textures, PlayerEntity *player)
    : GameObject(world, textures, ObjectType::EMP)
{
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
            // m_collider->removeObject(*this);

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
    // auto enemies = m_collider->findNearestObjects(ObjectType::Enemy, m_pos, m_explosion_radius);
    // for (auto enemy : enemies)
    // {
    //     static_cast<Enemy *>(enemy)->m_deactivated = true;
    //     static_cast<Enemy *>(enemy)->m_deactivated_time = 2.0f;
    // }
    // auto players = m_collider->findNearestObjects(ObjectType::Player, m_pos, m_explosion_radius);
    // for (auto player : players)
    // {
    //     static_cast<PlayerEntity *>(player)->m_deactivated_time = 2.0f;
    // }
}

void EMP::draw(LayersHolder &layers)
{

    auto &target = layers.getCanvas("Unit");

    m_texture_rect.m_tex_rect = m_animation->getCurrentTextureRect();

    if (!m_is_ticking)
    {

        m_texture_rect.setScale(2 * m_explosion_radius, 2 * m_explosion_radius);
    }
    m_texture_rect.setPosition(m_pos);
    m_texture_rect.setRotation(m_angle);
    target.drawSprite(m_texture_rect);
}

ExplosionAnimation::ExplosionAnimation(GameWorld *world, TextureHolder &textures, PlayerEntity *player)
    : GameObject(world, textures, ObjectType::Explosion)
{

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

void ExplosionAnimation::draw(LayersHolder &layers)
{
    auto &target = layers.getCanvas("Unit");

    Sprite rect;
    rect.setTexture(*m_textures->get("Explosion"));
    rect.m_tex_rect = m_animation->getCurrentTextureRect();
    rect.m_color = {255, 255, 255, 155};

    rect.setPosition(m_pos);
    rect.setRotation(m_angle);
    rect.setScale(2 * m_explosion_radius, 2 * m_explosion_radius);
    target.drawSprite(rect);
}

Heart::Heart(GameWorld *world, TextureHolder &textures, PlayerEntity *player, Pickup type)
    : GameObject(world, textures, ObjectType::Heart)
{
}

Heart::~Heart() {}

void Heart::update(float dt)
{
    m_pos += m_vel * dt;
}
void Heart::onCreation()
{
    CollisionComponent c_comp;
    Polygon shape = {4};
    m_size = 6.;

    shape.setScale(m_size / 2.);
    c_comp.shape.convex_shapes.push_back(shape);
    c_comp.type = ObjectType::Heart;
    m_world->m_systems.add(c_comp, getId());

    if (rand() % 2 == 0)
    {
        setPickupType(Pickup::Heart);
    }
    else
    {
        setPickupType(Pickup::Fuel);
    }
}

void Heart::onDestruction()
{
    auto &effect = m_world->addVisualEffect(EffectType::ParticleEmiter);
    effect.setPosition(m_pos);
    effect.setLifeTime(2.f);
}

void Heart::setPickupType(Pickup type)
{

    auto &c_comp = m_world->m_systems.get<CollisionComponent>(getId());

    if (type == Pickup::Heart)
    {
        c_comp.on_collision = [this](int id, auto type)
        {
            m_world->m_systems.get<HealthComponent>(id).hp += 2;
        };
    }
    else if (type == Pickup::Fuel)
    {
        c_comp.on_collision = [this](int id, auto type)
        {
            p_player->m_fuel += 2;
        };
    }

    m_pickup_type = type;
}
void Heart::onCollisionWith(GameObject &obj, CollisionData &c_data)
{
    if (obj.getType() == ObjectType::Player)
    {
        kill();
    }
}

void Heart::draw(LayersHolder &layers)
{
    auto &target = layers.getCanvas("Unit");

    Sprite rect;
    if (m_pickup_type == Pickup::Heart)
    {
        rect.setTexture(*m_textures->get("Heart"));
    }
    else if (m_pickup_type == Pickup::Fuel)
    {
        rect.setTexture(*m_textures->get("Fuel"));
    }
    rect.setPosition(m_pos);
    rect.setScale(m_size / 2.f);
    target.drawSprite(rect);
}

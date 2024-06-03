#include "Attacks.h"

#include <SFML/Graphics/RenderTarget.hpp>

#include "../CollisionSystem.h"
#include "../ResourceManager.h"
#include "../GameWorld.h"
#include "../Animation.h"

#include "Enemy.h"
#include "Player.h"

Bullet::Bullet(GameWorld *world, TextureHolder &textures,
                 PlayerEntity *player)
    : m_player(player), GameObject(world, textures, ObjectType::Bullet)
{
    m_collision_shape = std::make_unique<Polygon>(4);
    m_collision_shape->setScale({1, 1});
}

float Bullet::getTime() const
{
    return m_time;
}

Bullet::~Bullet() {}

void Bullet::update(float dt)
{
    auto dr_to_target = m_player->getPosition() - m_pos;
    auto acc = max_vel * dr_to_target / norm(dr_to_target) - m_vel;

    truncate(acc, max_acc);
    m_vel += acc * dt;
    truncate(m_vel, max_vel);
    m_pos += m_vel * dt;

    m_life_time -= dt;
    if (m_life_time < 0.f)
    {
        kill();
    }

    m_time += dt;
    if (m_time > 0.1f)
    {
        m_time = 0.f;
        m_past_positions.push_front(m_pos);
    }
    if (m_past_positions.size() > 5)
    {
        m_past_positions.pop_back();
    }
}

void Bullet::onCollisionWith(GameObject &obj, CollisionData &c_data)
{
    switch (obj.getType())
    {
    case ObjectType::Enemy:
    {
        if (m_life_time < 9.5f)
        {
            kill();
        }
        break;
    }
    case ObjectType::Meteor:
    {
        kill();
        break;
    }
    case ObjectType::Player:
    {
        kill();
        break;
    }
    case ObjectType::Laser:
        kill();
        break;
    }
}

void Bullet::onCreation()
{
}

void Bullet::onDestruction()
{
}

void Bullet::draw(sf::RenderTarget &target)
{

    sf::RectangleShape rect;
    rect.setOrigin({0.5, 0.5});
    rect.setPosition(m_pos);
    rect.setRotation(dir2angle(m_vel));
    rect.setSize({1., 1.});
    rect.setFillColor(sf::Color::Red);
    target.draw(rect);

    int alpha = 255;
    for (auto pos : m_past_positions)
    {
        alpha -= 255 / m_past_positions.size();
        alpha = std::max(0, alpha);
        rect.setPosition(pos);
        rect.setFillColor(sf::Color(255, 0, 0, alpha));
        target.draw(rect);
    }
}

Bomb::Bomb(GameWorld *world, TextureHolder &textures,
             Collisions::CollisionSystem &neighbour_searcher)
    : m_neighbour_searcher(&neighbour_searcher), GameObject(world, textures, ObjectType::Bomb)
{
    m_collision_shape = std::make_unique<Polygon>(4);
    m_collision_shape->setScale({2, 2});
    m_rigid_body = std::make_unique<RigidBody>();
    m_rigid_body->mass = 0.000001f;
    m_rigid_body->angle_vel = 0.000001f;
    m_rigid_body->inertia = 0.000001f;

    auto texture_size = static_cast<sf::Vector2i>(m_textures.get(Textures::ID::Bomb).getSize());

    m_animation = std::make_unique<Animation>(texture_size,
                                              7, 2, m_life_time);
}

Bomb::~Bomb() {}

void Bomb::update(float dt)
{
    m_life_time -= dt;

    m_vel -= 0.05f * m_vel;

    if (m_life_time < 0.f)
    {
        kill();
    }

    m_animation->update(dt);

    m_pos += m_vel * dt;
}

void Bomb::onCollisionWith(GameObject &obj, CollisionData &c_data)
{
}

void Bomb::onCreation()
{
}

void Bomb::onDestruction()
{

    auto meteors = m_neighbour_searcher->findNearestObjects(ObjectType::Meteor, m_pos, m_explosion_radius);
    for (auto p_meteor : meteors)
    {
        auto dr_to_center = m_pos - p_meteor->getPosition();
        auto dist_to_center = norm(dr_to_center);
        auto impulse_dir = -dr_to_center / dist_to_center;

        auto distance_factor = 1 - dist_to_center / m_explosion_radius;
        if (distance_factor > 0)
        {
            p_meteor->m_vel += distance_factor * impulse_dir * 5.f;
            ;
        }
    }

    auto &explosion = m_world->addObject(ObjectType::Explosion);
    explosion.setPosition(m_pos);
}

void Bomb::draw(sf::RenderTarget &target)
{
    sf::RectangleShape rect;
    rect.setTexture(&m_textures.get(Textures::ID::Bomb));
    rect.setTextureRect(m_animation->getCurrentTextureRect());

    rect.setOrigin({1, 1});
    rect.setPosition(m_pos);
    rect.setRotation(m_angle);
    rect.setSize({2, 2});
    target.draw(rect);
}

Laser::Laser(GameWorld *world, TextureHolder &textures,
               Collisions::CollisionSystem &neighbour_searcher)
    : m_neighbour_searcher(&neighbour_searcher), GameObject(world, textures, ObjectType::Laser)
{
    m_collision_shape = std::make_unique<Polygon>(4);

    m_is_bloomy = true;
}

Laser::~Laser() {}

void Laser::update(float dt)
{
    m_life_time -= dt;

    m_length += 3.f;
    if (m_owner)
    {
        m_pos = m_owner->getPosition();
    }

    auto hit = m_neighbour_searcher->findClosestIntesection(ObjectType::Meteor, m_pos, angle2dir(m_angle), m_length);

    m_length = dist(hit, m_pos);
    setSize({m_length, m_width});
    //! m_pos of laser is special, it is starting position not center so we set it manually
    m_collision_shape->setPosition(m_pos + m_length / 2.f * angle2dir(m_angle));

    if (m_life_time < 0.f)
    {
        kill();
    }
}

void Laser::onCollisionWith(GameObject &obj, CollisionData &c_data)
{
}

void Laser::onCreation()
{
}

void Laser::onDestruction()
{
}

void Laser::draw(sf::RenderTarget &target)
{

    sf::VertexArray verts;
    verts.resize(4);
    verts.setPrimitiveType(sf::Quads);

    sf::RectangleShape rect;
    rect.setOrigin({0, m_width / 2.f});
    rect.setPosition(m_pos);
    rect.setRotation(m_angle);
    rect.setSize({m_length, m_width});
    rect.setFillColor(sf::Color{255, 255, 50, 255});
    target.draw(rect);
}
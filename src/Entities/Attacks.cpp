#include "Attacks.h"

#include "../DrawLayer.h"
#include "../CollisionSystem.h"
// #include "../ResourceManager.h"
#include "../GameWorld.h"
#include "../Animation.h"

#include "Enemy.h"
#include "Player.h"

std::unordered_map<BulletType, std::string> Bullet::m_type2shader_id = {{BulletType::Lightning, "lightningBolt"},
                                                                        {BulletType::Fire, "fireBolt"}};

Bullet::Bullet(GameWorld *world, TextureHolder &textures, PlayerEntity *player)
    : GameObject(world, textures, ObjectType::Bullet)
{
    m_collision_shape = std::make_unique<Polygon>(4);
    m_collision_shape->setScale(1, 1);
}

float Bullet::getTime() const
{
    return m_time;
}

void Bullet::setTarget(GameObject *new_target)
{
    m_target = new_target;
}

GameObject *Bullet::getTarget() const
{
    return m_target;
}

Bullet::~Bullet() {}

void Bullet::update(float dt)
{
    utils::Vector2f acc = 0;
    if (m_target)
    {
        auto dr_to_target = m_target->getPosition() - m_pos;
        acc = m_max_vel * dr_to_target / norm(dr_to_target) - m_vel;
    }

    utils::truncate(acc, m_max_acc);
    m_vel += acc * dt;
    utils::truncate(m_vel, m_max_vel);
    m_pos += m_vel * dt;

    m_time += dt;
    if (m_time >= m_life_time)
    {
        kill();
    }

    m_tail_timer += dt;
    //! remember past positions to draw a tail
    if (m_tail_timer > 0.1f)
    {
        m_tail_timer = 0.f;
        m_past_positions.push_front(m_pos);
    }
    if (m_past_positions.size() > 4)
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
        if (m_life_time < 10.f)
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

void Bullet::setBulletType(BulletType type)
{
    m_type = type;
}

void Bullet::draw(LayersHolder &layers)
{

    auto &target = layers.getCanvas("Unit");

    Sprite rect;
    rect.setTexture(*m_textures.get("Bomb"));
    rect.setPosition(m_pos);
    rect.setRotation(glm::radians(dir2angle(m_vel)));
    rect.setScale(5., 5.);
    rect.m_color = {255, 0, 0, 255};

    assert(m_type2shader_id.count(m_type) > 0);
    auto shader_id = m_type2shader_id.at(m_type);
    target.drawSprite(rect, shader_id);

    int alpha = 255;
    for (auto pos : m_past_positions)
    {
        alpha -= 255 / m_past_positions.size();
        alpha = std::max(0, alpha);
        rect.setPosition(pos);
        rect.m_color = {255, 0, 0, static_cast<unsigned char>(alpha)};
        rect.setScale(3.5, 3.5);
        target.drawSprite(rect, shader_id);
    }
}

Bomb::Bomb(GameWorld *world, TextureHolder &textures,
           Collisions::CollisionSystem &neighbour_searcher)
    : m_neighbour_searcher(&neighbour_searcher), GameObject(world, textures, ObjectType::Bomb)
{
    m_collision_shape = std::make_unique<Polygon>(4);
    m_collision_shape->setScale(2, 2);
    // m_rigid_body = std::make_unique<RigidBody>();
    // m_rigid_body->mass = 0.01f;
    // // m_rigid_body->angle_vel = 0.0;
    // m_rigid_body->inertia = 0.001f;

    auto texture_size = static_cast<utils::Vector2i>(m_textures.get("Bomb")->getSize());

    m_animation = std::make_unique<Animation>(texture_size,
                                              7, 2, m_life_time);
}

Bomb::~Bomb() {}

void Bomb::update(float dt)
{

    auto old_speed = utils::norm(m_vel);
    auto new_speed = old_speed -  m_acc * dt;
    new_speed = std::max(0.f, new_speed);
    
    if(old_speed > 0.)
    {
        m_vel *= new_speed / old_speed;
    }

    m_life_time -= dt;
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
        }
    }

    auto &explosion = static_cast<Explosion&>(m_world->addObject(ObjectType::Explosion));
    explosion.setPosition(m_pos);
    explosion.m_is_expanding = true;
    explosion.m_max_explosion_radius = 25.;
}

void Bomb::draw(LayersHolder &layers)
{
    auto &target = layers.getCanvas("Unit");
    Sprite rect;
    rect.setTexture(*m_textures.get("Bomb"));
    rect.m_tex_rect = m_animation->getCurrentTextureRect();

    // rect.setOrigin(1, 1);
    rect.setPosition(m_pos);
    rect.setRotation(m_angle);
    rect.setScale(2, 2);
    target.drawSprite(rect, "Instanced");
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
    m_time += dt;

    m_length = m_max_length * (m_time / m_life_time);
    if (m_owner)
    {
        m_pos = m_owner->getPosition();
        if(m_rotates_with_owner)
        {
            m_angle = m_owner->getAngle();
        }
    }

    auto hit = m_neighbour_searcher->findClosestIntesection(ObjectType::Meteor, m_pos, utils::angle2dir(m_angle), m_length);

    m_length = dist(hit, m_pos);
    setSize({m_length / sqrtf(2), m_width});
    //! m_pos of laser is special, it is starting position not center so we set it manually
    m_collision_shape->setPosition(m_pos + m_length / 2.f * utils::angle2dir(m_angle));

    if (m_time > m_life_time)
    {
        kill();
    }
}

void Laser::onCollisionWith(GameObject &obj, CollisionData &c_data)
{
    switch (obj.getType())
    {
    case ObjectType::Enemy:
    {
        if (&obj != getOwner())
        {
            static_cast<Enemy&>(obj).m_health -= m_max_dmg;
        }
        break;
    }
    case ObjectType::Player:
    {
        if (&obj != getOwner())
        {
            static_cast<PlayerEntity&>(obj).health -= m_max_dmg;
        }
        break;
    }
    }
}

void Laser::onCreation()
{
}

void Laser::onDestruction()
{
}

void Laser::draw(LayersHolder &layers)
{

    auto &shiny_target = layers.getCanvas("Bloom");
    auto &target = layers.getCanvas("Bloom");

    Sprite rect;
    rect.setTexture(*m_textures.get("BoosterPurple"));
    rect.setPosition(m_collision_shape->getPosition());
    rect.setRotation(glm::radians(m_angle));
    rect.setScale(m_length / 2., m_width / 2.);
    rect.m_color = {255, 255, 50, 255};
    target.drawSprite(rect, "laser");
}

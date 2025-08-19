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
    m_size = {2.5f};
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

void Bullet::update(float dt)
{
    m_time += dt;

    utils::truncate(m_acc, m_max_acc);
    m_vel += m_acc * dt;
    utils::truncate(m_vel, m_max_vel);
    m_pos += m_vel * dt;

    m_acc = {0.f};
  
}

void Bullet::onCollisionWith(GameObject &obj, CollisionData &c_data)
{
    if(m_collision_resolvers.contains(obj.getType()))
    {
        m_collision_resolvers.at(obj.getType())(obj, c_data);
        return;
    }

    switch (obj.getType())
    {
    case ObjectType::Enemy:
    {
        if (m_shooter != &obj)
        {
            m_world->p_messenger->send(DamageReceivedEvent{ObjectType::Bullet, getId(),
                                                           ObjectType::Enemy, obj.getId(), 3.});
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
        m_world->p_messenger->send(DamageReceivedEvent{ObjectType::Bullet, getId(),
                                                       ObjectType::Player, obj.getId(), 3.});
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
    CollisionComponent c_comp;
    c_comp.type = ObjectType::Bullet;
    c_comp.shape.convex_shapes.emplace_back(8);
    
    TimedEvent die_on_timeout = {10.f, [this](float t, int count){kill();}, 1};
    TimedEventComponent time_comp;
    time_comp.addEvent(die_on_timeout);

    m_world->m_systems.addEntity(getId(), c_comp, time_comp);
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

    m_sprite.setTexture(*m_textures->get("Bomb"));
    m_sprite.setPosition(m_pos);
    m_sprite.setRotation(glm::radians(dir2angle(m_vel)));
    m_sprite.setScale(m_size / 2.f);
    m_sprite.m_color = {255, 0, 0, 255};

    assert(m_type2shader_id.count(m_type) > 0);
    auto shader_id = m_type2shader_id.at(m_type);
    target.drawSprite(m_sprite, shader_id);


    // int alpha = 255;
    // for (auto pos : m_past_positions)
    // {
    //     alpha -= 255 / m_past_positions.size();
    //     alpha = std::max(0, alpha);
    //     rect.setPosition(pos);
    //     rect.m_color = {255, 0, 0, static_cast<unsigned char>(alpha)};
    //     rect.setScale(m_size * 0.69);
    //     target.drawSprite(rect, shader_id);
    // }
}

Bomb::Bomb(GameWorld *world, TextureHolder &textures, PlayerEntity *player)
    : GameObject(world, textures, ObjectType::Bomb)
{
    // m_rigid_body = std::make_unique<RigidBody>();
    // m_rigid_body->mass = 0.01f;
    // // m_rigid_body->angle_vel = 0.0;
    // m_rigid_body->inertia = 0.001f;

    auto texture_size = static_cast<utils::Vector2i>(m_textures->get("Bomb")->getSize());

    m_animation = std::make_unique<Animation>(texture_size,
                                              7, 2, m_life_time);
}

void Bomb::update(float dt)
{

    auto old_speed = utils::norm(m_vel);
    auto new_speed = old_speed - m_acc * dt;
    new_speed = std::max(0.f, new_speed);

    if (old_speed > 0.)
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

    auto meteors = m_world->getCollisionSystem().findNearestObjects(ObjectType::Meteor, m_pos, m_explosion_radius);
    for (auto p_meteor : meteors)
    {
        auto dr_to_center = m_pos - p_meteor->shape.convex_shapes[0].getPosition();
        auto dist_to_center = norm(dr_to_center);
        auto impulse_dir = -dr_to_center / dist_to_center;

        auto distance_factor = 1 - dist_to_center / m_explosion_radius;
        if (distance_factor > 0)
        {
            // p_meteor->m_vel += distance_factor * impulse_dir * 5.f;
        }
    }

    auto &explosion = m_world->addObject2<Explosion>();
    explosion.setPosition(m_pos);
    explosion.m_is_expanding = true;
    explosion.m_max_explosion_radius = 25.;
}

void Bomb::draw(LayersHolder &layers)
{
    auto &target = layers.getCanvas("Unit");
    Sprite rect;
    rect.setTexture(*m_textures->get("Bomb"));
    rect.m_tex_rect = m_animation->getCurrentTextureRect();

    // rect.setOrigin(1, 1);
    rect.setPosition(m_pos);
    rect.setRotation(m_angle);
    rect.setScale(2, 2);
    target.drawSprite(rect);
}

Laser::Laser(GameWorld *world, TextureHolder &textures, PlayerEntity *player)
    : GameObject(world, textures, ObjectType::Laser)
{
}

Laser::~Laser() 
{

}

void Laser::stopAgainst(ObjectType type)
{
    auto dir = utils::angle2dir(m_angle);
    auto hit = m_world->getCollisionSystem().findClosestIntesection(type, m_pos, utils::angle2dir(m_angle), m_length);

    m_length = dist(hit, m_pos);
    setSize({m_length, m_width});
    //! m_pos of laser is special, it is starting position not center so we set it manually
    // setPosition(m_pos + m_length / 2.f * utils::angle2dir(m_angle));
}

void Laser::update(float dt)
{
    m_time += dt;
    m_updater(dt);

    m_width += m_max_width * (dt / m_life_time);
    m_length += m_max_length * (dt / m_life_time);
    
    for(auto& stop_type : m_stopping_types)
    {
        stopAgainst(stop_type);
    }
    if (m_parent)
    {
        if (m_rotates_with_owner)
        {
            m_angle = m_parent->getAngle();
        }
        m_pos = m_parent->getPosition() + m_offset + utils::angle2dir(m_angle) * m_length / 2.;
    }
    setSize({m_length, m_width});

    if (m_time > m_life_time)
    {
        kill();
    }
}

void Laser::onCollisionWith(GameObject &obj, CollisionData &c_data)
{
    if (!obj.isParentOf(this))
    {
        DamageReceivedEvent dmg = {obj.getType(), obj.getId(), ObjectType::Laser, obj.getId(), m_max_dmg};
        m_world->p_messenger->send(dmg);
    }
}

void Laser::onCreation()
{
    Polygon shape = {4};
    CollisionComponent c_comp = {std::vector<Polygon>{shape}, ObjectType::Laser};
    m_world->m_systems.add(c_comp, getId());
}

void Laser::onDestruction()
{
    GameObject::onDestruction();
}

void Laser::draw(LayersHolder &layers)
{

    auto &shiny_target = layers.getCanvas("Bloom");
    auto &target = layers.getCanvas("Bloom");

    Sprite rect;
    rect.setTexture(*m_textures->get("BoosterPurple"));
    rect.setPosition(getPosition());
    rect.setRotation(glm::radians(m_angle));
    rect.setScale(m_length / 2., m_width / 2.);
    rect.m_color = m_laser_color;
    shiny_target.drawSprite(rect, "laser");
    // target.drawCricleBatched(m_pos, 2, {1,0,0,1});
    // target.drawCricleBatched(m_pos + utils::angle2dir(m_angle) * m_length/2., 2, {1,0,1,1});
}

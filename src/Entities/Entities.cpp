#include "Entities.h"

#include "../DrawLayer.h"
#include "../CollisionSystem.h"
// #include "../GridNeighbourSearcher.h"
#include "../BehaviourBase.h"
// #include "../ResourceManager.h"
// #include "../ResourceHolder.h"
// #include "../Geom"
#include "../GameWorld.h"
#include "../Animation.h"
#include "../Polygon.h"
#include "../Utils/RandomTools.h"

#include "Enemy.h"
#include "Player.h"
#include "Attacks.h"


Explosion::Explosion(GameWorld *world, TextureHolder &textures)
    : GameObject(world, textures, ObjectType::Explosion)
{
    m_collision_shape = std::make_unique<Polygon>(4);

    auto texture_size = static_cast<utils::Vector2i>(m_textures.get("Explosion2")->getSize());

    m_animation = std::make_unique<Animation>(texture_size,
                                              12, 1, m_life_time, 1, true);
}

Explosion::~Explosion() {}

void Explosion::setType(std::string texture_id)
{
    //! TODO: ADD some animation manager
    if (texture_id == "Explosion")
    {
        auto texture_size = static_cast<utils::Vector2i>(m_textures.get(texture_id)->getSize());
        m_animation = std::make_unique<Animation>(texture_size,
                                                  12, 1, m_life_time / 0.016f, 1, true);
    }
    else if (texture_id == "Explosion2")
    {
        auto texture_size = static_cast<utils::Vector2i>(m_textures.get("Explosion2")->getSize());
        m_animation = std::make_unique<Animation>(texture_size,
                                                  4, 4, m_life_time / 0.016f, 1, false);
    }
}

void Explosion::update(float dt)
{
    m_time += dt;
    if (m_time > m_life_time)
    {
        kill();
    }
    m_explosion_radius = m_time / m_life_time * m_max_explosion_radius;
    if (m_collision_shape)
    {
        m_collision_shape->setScale(2 * m_explosion_radius, 2 * m_explosion_radius);
    }
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
    Sprite rect;
    rect.setTexture(*m_textures.get("Explosion2"));
    rect.m_tex_rect = m_animation->getCurrentTextureRect();
    rect.m_color = {255, 255, 255, 255};

    // rect.setOrigin({m_explosion_radius, m_explosion_radius});
    rect.setPosition(m_pos);
    rect.setRotation(m_angle);
    rect.setScale(2 * m_explosion_radius, 2 * m_explosion_radius);
    target.drawSprite(rect, "Instanced");
}

EMP::EMP(GameWorld *world, TextureHolder &textures, Collisions::CollisionSystem *collider)
    : m_collider(collider), GameObject(world, textures, ObjectType::EMP)
{
    m_collision_shape = std::make_unique<Polygon>(8);
    m_collision_shape->setScale(2, 2);
    auto texture_size = static_cast<utils::Vector2i>(m_textures.get("Bomb")->getSize());

    m_animation = std::make_unique<Animation>(texture_size,
                                              7, 2, m_life_time, 0);

    m_texture_rect.setTexture(*m_textures.get("Bomb"));
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

            auto texture_size = static_cast<utils::Vector2i>(m_textures.get("Emp")->getSize());
            m_texture_rect.setTexture(*m_textures.get("Emp"));
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
    target.drawSprite(m_texture_rect, "Instanced");
}

ExplosionAnimation::ExplosionAnimation(GameWorld *world, TextureHolder &textures)
    : GameObject(world, textures, ObjectType::Explosion)
{
    m_collision_shape = std::make_unique<Polygon>(4);
    m_collision_shape->setScale(2 * m_explosion_radius, 2 * m_explosion_radius);

    auto texture_size = static_cast<utils::Vector2i>(m_textures.get("Explosion")->getSize());

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
    rect.setTexture(*m_textures.get("Explosion"));
    rect.m_tex_rect = m_animation->getCurrentTextureRect();
    rect.m_color = {255, 255, 255, 155};

    rect.setPosition(m_pos);
    rect.setRotation(m_angle);
    rect.setScale(2 * m_explosion_radius, 2 * m_explosion_radius);
    target.drawSprite(rect, "Instanced");
}

Heart::Heart(GameWorld *world, TextureHolder &textures)
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
    rect.setTexture(*m_textures.get("Heart"));
    rect.setPosition(m_pos);
    rect.setRotation(dir2angle(m_vel));
    rect.setScale(3., 3.);
    target.drawSprite(rect, "Instanced");
}

SpaceStation::SpaceStation(GameWorld *world, TextureHolder &textures)
    : GameObject(world, textures, ObjectType::SpaceStation)
{
    m_collision_shape = std::make_unique<Polygon>(4);
    m_size = {10, 10};
    m_collision_shape->setScale(m_size.x, m_size.y);
    m_rigid_body = std::make_unique<RigidBody>();
    m_rigid_body->mass = 5000000.f;
    m_rigid_body->inertia = 50000000.f;
    m_rigid_body->angle_vel = 0;
}

SpaceStation::~SpaceStation() {}

void SpaceStation::update(float dt)
{

    m_time += dt;
    if (m_time > m_spawn_timer && m_produced_ships.size() < 3)
    {
        m_time = 0.f;
        auto &new_enemy = static_cast<Enemy &>(m_world->addObject(ObjectType::Enemy));
        new_enemy.setBehaviour();
        float rand_angle = randf(-180, 180);

        new_enemy.setPosition(m_pos + 10.f * utils::angle2dir(rand_angle));
        new_enemy.m_vel = 200.f * utils::angle2dir(rand_angle);
        m_produced_ships.push_back(&new_enemy);
    }

    std::vector<int> to_remove;
    int i = 0;
    for (auto &ship : m_produced_ships)
    {
        if (ship->isDead())
        {
            to_remove.push_back(i);
        }
        i++;
    }
    while (!to_remove.empty())
    {
        m_produced_ships.erase(m_produced_ships.begin() + to_remove.back());
        to_remove.pop_back();
    }

    if (m_health < 0.f)
    {
        kill();
    }
}

void SpaceStation::onCollisionWith(GameObject &obj, CollisionData &c_data)
{
    switch (obj.getType())
    {
    case ObjectType::Bullet:
    {
        obj.kill();
        m_health--;
        break;
    }
    case ObjectType::Meteor:
    {

        break;
    }
    case ObjectType::Player:
    {

        break;
    }
    case ObjectType::Explosion:
    {
        auto dr_to_center = m_pos - obj.getPosition();
        auto dist_to_center = norm(dr_to_center);
        auto impulse_dir = dr_to_center / dist_to_center;

        auto alpha = 1 - dist_to_center / obj.getCollisionShape().getScale().x;
        // m_health -= 0.1f*alpha;
        break;
    }
    case ObjectType::Laser:
        m_health -= 0.5f;
        break;
    }
}

void SpaceStation::onCreation()
{
}

void SpaceStation::onDestruction()
{
}

void SpaceStation::draw(LayersHolder& layers)
{

    auto& target = layers.getCanvas("Unit"); 

    Sprite rect;
    rect.setPosition(m_pos);
    rect.setRotation(dir2angle(m_vel));
    rect.setTexture(*m_textures.get("Station"));
    rect.setScale(m_size.x, m_size.y);
    // rect.setFillColor(sf::Color::Red);
    target.drawSprite(rect, "Instanced");

    // sf::RectangleShape health_rect;

    // float alpha_health = m_health / m_max_health;
    // float h_rect_size = m_size.x * alpha_health;

    // health_rect.setPosition(m_pos + utils::Vector2f(-m_size.x / 2.f, -m_size.y / 2.f * 3.f));
    // health_rect.setFillColor(sf::Color::Red);

    // health_rect.setSize({h_rect_size, 1.f});
    // target.draw(health_rect);
}

Boss::Boss(GameWorld *world, TextureHolder &textures, PlayerEntity *player)
    : m_player(player),
      GameObject(world, textures, ObjectType::Boss)
{
    m_collision_shape = std::make_unique<Polygon>(4);
    m_collision_shape->setScale(10, 10);
}

Boss::~Boss() {}

void Boss::update(float dt)
{

    if (dist(m_pos, m_target_pos) < m_vision_radius / 10.f)
    {
        m_target_pos = m_pos + utils::angle2dir(randf(0, 360)) * 30.f;
    }

    if (m_state == State::Patroling)
    {
        if (dist(m_pos, m_player->getPosition()) < m_vision_radius)
        {
            m_state = State::Shooting;
        }
    }
    else if (m_state == State::Shooting)
    {
        m_shooting_timer += dt;
        if (m_shooting_timer > m_shooting_cooldown)
        {
            m_shooting_timer = 0;
            shootAtPlayer();
            m_state = State::ThrowingBombs;
        }
    }
    else if (m_state == State::ThrowingBombs)
    {
        m_shooting_timer += dt;
        if (m_shooting_timer > m_bombing_cooldown)
        {
            m_shooting_timer = 0;
            throwBombsAtPlayer();
            m_bomb_count++;
        }
        if (m_bomb_count > 6)
        {
            m_bomb_count = 0;
            m_state = State::ShootingLasers;
            max_vel = 60.f;
        }
    }
    else if (m_state == State::ShootingLasers)
    {
        m_shooting_timer += dt;
        if (m_shooting_timer > m_lasering_cooldown)
        {
            m_shooting_timer = 0;
            shootLasers();
            m_state = State::Shooting;
            max_vel = 30.f;
            m_acc *= 0.f;
            m_vel *= 0.f;
        }
    }

    if (m_state != State::Patroling && dist(m_pos, m_player->getPosition()) > 1.5f * m_vision_radius)
    {
        m_state = State::Patroling;
    }

    auto dr_to_target = m_target_pos - m_pos;
    m_acc = max_vel * dr_to_target;

    truncate(m_acc, max_vel);
    m_vel += m_acc * dt;

    truncate(m_vel, max_vel);
    m_pos += (m_vel + m_impulse) * dt;
    if (m_health < 0.f)
    {
        kill();
    }
    m_impulse *= 0.f;
    m_acc *= 0.f;
}

void Boss::shootAtPlayer()
{

    float bullet_angle = utils::dir2angle(m_pos - m_player->getPosition());
    for (int i = -2; i <= 2; ++i)
    {
        auto angle = bullet_angle + i * (30.f);
        auto &bullet = m_world->addObject(ObjectType::Bullet);
        auto dir = utils::angle2dir(angle);
        bullet.setPosition(m_pos + 12.f * dir);
        bullet.m_vel = dir * 30.f;
    }
}
void Boss::throwBombsAtPlayer()
{
    float bullet_angle = utils::dir2angle(m_pos - m_player->getPosition());

    auto angle = bullet_angle + m_bomb_count * (30.f);
    auto &bomb = m_world->addObject(ObjectType::Bomb);
    auto dir = utils::angle2dir(angle);
    bomb.setPosition(m_pos + 12.f * dir);
    bomb.m_vel = dir * 100.f;
    m_bomb_count++;
}
void Boss::shootLasers()
{
    for (int i = 0; i <= 10; ++i)
    {
        auto angle = i / 10.f * 360.f;
        auto &laser = m_world->addObject(ObjectType::Laser);
        auto dir = utils::angle2dir(angle);
        laser.setPosition(m_pos + 12.f * dir);
        laser.setAngle(angle);
    }
}

void Boss::onCollisionWith(GameObject &obj, CollisionData &c_data)
{
    switch (obj.getType())
    {
    case ObjectType::Bullet:
    {
        auto &bullet = static_cast<Bullet &>(obj);
        m_health--;
        break;
    }
    case ObjectType::Meteor:
    {
        // m_health--;
        break;
    }
    case ObjectType::Explosion:
    {
        auto &explosion = static_cast<Explosion &>(obj);
        auto dr_to_center = m_pos - obj.getPosition();
        auto dist_to_center = norm(dr_to_center);
        auto impulse_dir = dr_to_center / dist_to_center;

        auto time_factor = explosion.getTimeLeftFraciton();
        auto distance_factor = 1 - dist_to_center / obj.getCollisionShape().getScale().x;
        if (distance_factor > 0)
        {
            // m_impulse += time_factor * distance_factor * impulse_dir * 100.f;
            m_health -= distance_factor * time_factor * 0.1f;
        }
        break;
    }
    case ObjectType::Laser:
        m_health -= 0.1f;
        break;
    }
}

void Boss::onCreation()
{
    m_target_pos = m_pos;
}
void Boss::onDestruction()
{
    auto &new_explosion = static_cast<Explosion &>(m_world->addObject(ObjectType::Explosion));
    new_explosion.removeCollider();
    new_explosion.setPosition(m_pos);
    new_explosion.m_explosion_radius = 10.f;
    new_explosion.setType("Explosion2");

    // SoundManager::play(0);
}

void Boss::draw(LayersHolder& layers)
{

    auto &target = layers.getCanvas("Unit");
    Sprite rect;

    rect.setTexture(*m_textures.get("BossShip"));

    rect.setPosition(m_pos);
    rect.setRotation(glm::radians(dir2angle(m_vel)));
    rect.setScale(10, 10);

    target.drawSprite(rect, "Instanced");

    Sprite booster;
    utils::Vector2f booster_size = {4, 2};

    booster.setTexture(*m_textures.get("BoosterPurple"));
    booster.setScale(booster_size.x, booster_size.y);
    booster.setPosition(m_pos - m_vel / norm(m_vel) * rect.getScale().y);
    booster.setRotation(rect.getRotation());
    target.drawSprite(booster, "Instanced");
}

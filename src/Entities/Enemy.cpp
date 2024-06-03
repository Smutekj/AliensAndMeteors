#include "Enemy.h"

#include <SFML/Graphics/RenderTarget.hpp>

#include "../CollisionSystem.h"
#include "../GridNeighbourSearcher.h"
#include "../BehaviourBase.h"
#include "../ResourceManager.h"
#include "../ResourceHolder.h"
#include "../GameWorld.h"
#include "../Animation.h"

#include "Entities.h"
#include "Attacks.h"
#include "Player.h"

Enemy::Enemy(GameWorld *world, TextureHolder &textures,
             Collisions::CollisionSystem &collider, GridNeighbourSearcher &m_ns, PlayerEntity *player)
    : m_collision_system(&collider), m_neighbour_searcher(&m_ns), m_player(player), GameObject(world, textures, ObjectType::Enemy)
{
    m_collision_shape = std::make_unique<Polygon>(4);
    m_collision_shape->setScale({4, 4});
    setBehaviour();
    m_target_pos = player->getPosition();
}

Enemy::~Enemy() {}

void Enemy::update(float dt)
{
    m_neighbour_searcher->moveEntity(*this);
    m_behaviour->update();

    boidSteering();
    avoidMeteors();

    if (m_deactivated)
    {
        m_deactivated_time -= dt;
        if (m_deactivated_time < 0)
        {
            m_deactivated = false;
        }
    }

    truncate(m_acc, max_acc);
    if (!m_deactivated)
    {
        m_vel += m_acc * dt;
    }

    truncate(m_vel, max_vel);
    m_pos += (m_vel + m_impulse) * dt;
    if (m_health < 0.f)
    {
        kill();
    }
    m_impulse *= 0.f;
    m_acc *= 0.f;
}

void Enemy::onCollisionWith(GameObject &obj, CollisionData &c_data)
{
    switch (obj.getType())
    {
    case ObjectType::Bullet:
    {
        auto &bullet = static_cast<Bullet &>(obj);
        if (bullet.getTime() > 1.5f)
        {
            m_health--;
        }
        break;
    }
    case ObjectType::Meteor:
    {
        m_health--;
        auto mvt = c_data.separation_axis;
        if (dot(mvt, m_vel) < 0.f)
        {
            m_vel -= 2.f * dot(mvt, m_vel) * mvt;
        }
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
            m_impulse += time_factor * distance_factor * impulse_dir * 100.f;
            m_health -= distance_factor * time_factor;
        }
        break;
    }
    case ObjectType::Laser:
    {
        if (static_cast<Laser &>(obj).getOwner() != this)
        {
            m_health -= 0.1f;
        }
        break;
    }
    }
}

void Enemy::onCreation()
{
    m_neighbour_searcher->insertEntity(*this);
}
void Enemy::onDestruction()
{
    m_neighbour_searcher->removeEntity(getId());

    auto &new_explosion = static_cast<Explosion &>(m_world->addObject(ObjectType::Explosion));
    new_explosion.removeCollider();
    new_explosion.setPosition(m_pos);
    new_explosion.m_explosion_radius = 4.f;
    new_explosion.setType(Textures::ID::Explosion2);

    SoundManager::play(0);
}

void Enemy::draw(sf::RenderTarget &target)
{
    sf::RectangleShape rect;

    rect.setTexture(&m_textures.get(Textures::ID::EnemyShip));

    rect.setOrigin({1.5, 1.5});
    rect.setPosition(m_pos);
    rect.setRotation(dir2angle(m_vel));
    rect.setSize({3, 3});
    // if(m_is_avoiding){ rect.setFillColor(sf::Color::Red);}

    target.draw(rect);

    sf::RectangleShape booster;
    sf::Vector2f booster_size = {4, 2};

    booster.setTexture(&m_textures.get(Textures::ID::BoosterPurple));
    booster.setSize(booster_size);
    booster.setOrigin(booster_size / 2.f);
    booster.setPosition(m_pos - m_vel / norm(m_vel) * rect.getSize().y);
    booster.setRotation(rect.getRotation());
    target.draw(booster);

    // sf::RectangleShape line;
    // line.setFillColor(sf::Color::Green);

    // for(auto pos : m_cm)
    // {
    //     auto dr = pos - m_pos;

    //     line.setPosition(m_pos);
    //     line.setRotation(dir2angle(dr));
    //     line.setSize({norm(dr), 1.f});
    //     line.setOrigin({0, 0.5f });
    //     target.draw(line);
    // }
}

void Enemy::avoidMeteors()
{

    const auto &r = m_pos;

    auto nearest_meteors =
        m_collision_system->findNearestObjects(ObjectType::Meteor, r, m_boid_radius);

    m_is_avoiding = false;
    m_cm.clear();
    sf::Vector2f avoid_force = {0, 0};
    for (auto *meteor : nearest_meteors)
    {
        auto &meteor_shape = meteor->getCollisionShape();

        auto r_meteor = meteor_shape.getPosition();
        auto radius_meteor = meteor_shape.getScale().x;
        auto dr_to_target = m_target_pos - m_pos;
        dr_to_target /= norm(dr_to_target);

        auto dr_to_meteor = (r_meteor - r) / norm(r - r_meteor);
        sf::Vector2f dr_norm = {dr_to_meteor.y, -dr_to_meteor.x};
        dr_norm /= norm(dr_norm);

        auto dist_to_meteor = dist(r, r_meteor);
        if (dist_to_meteor < 2. * radius_meteor)
        {
            m_is_avoiding = true;
            const auto angle = angleBetween(dr_to_meteor, dr_to_target);
            const float sign = 2 * (angle < 0) - 1;

            if (std::abs(angle) < 110)
            {
                m_cm.push_back(r_meteor);
                avoid_force += sign * dr_norm / (dist_to_meteor - radius_meteor / 2.f);
                avoid_force *= m_force_multipliers[Multiplier::AVOID];
            }
        }
    }
    // truncate(avoid_force, 500000.f);
    m_acc += avoid_force;
}

std::unordered_map<Multiplier, float> Enemy::m_force_multipliers = {
    {Multiplier::ALIGN, 0.f},
    {Multiplier::AVOID, 25000.f},
    {Multiplier::SCATTER, 10.f},
    {Multiplier::SEEK, 10.f}};
std::unordered_map<Multiplier, float> Enemy::m_force_ranges = {
    {Multiplier::ALIGN, 20.f},
    {Multiplier::AVOID, 30.f},
    {Multiplier::SCATTER, 30.f},
    {Multiplier::SEEK, 10.f}};

void Enemy::boidSteering()
{
    // auto neighbours = m_neighbour_searcher->getNeighboursOfExcept(m_pos, m_boid_radius, m_id);

    auto neighbours = m_collision_system->findNearestObjects(ObjectType::Enemy, m_pos, m_boid_radius);

    sf::Vector2f repulsion_force(0, 0);
    sf::Vector2f push_force(0, 0);
    sf::Vector2f scatter_force(0, 0);
    sf::Vector2f cohesion_force(0, 0);
    sf::Vector2f seek_force(0, 0);
    float n_neighbours = 0;
    float n_neighbours_group = 0;
    sf::Vector2f dr_nearest_neighbours(0, 0);
    sf::Vector2f average_neighbour_position(0, 0);

    sf::Vector2f align_direction = {0, 0};
    int align_neighbours_count = 0;

    const float scatter_multiplier = Enemy::m_force_multipliers[Multiplier::SCATTER];
    const float align_multiplier = Enemy::m_force_multipliers[Multiplier::ALIGN];
    const float seek_multiplier = Enemy::m_force_multipliers[Multiplier::SEEK];

    auto range_align = std::pow(Enemy::m_force_ranges[Multiplier::ALIGN], 2);
    auto range_scatter = std::pow(Enemy::m_force_ranges[Multiplier::SCATTER], 2);
    ;

    for (auto p_neighbour : neighbours)
    {
        if (p_neighbour == this)
        {
            continue;
        }
        auto &neighbour_boid = *p_neighbour;
        // if(ind_j == boid_ind){continue;}
        const auto dr = neighbour_boid.getPosition() - m_pos;
        const auto dist2 = norm2(dr);

        if (dist2 < range_align)
        {
            align_direction += neighbour_boid.m_vel;
            align_neighbours_count++;
        }

        if (dist2 < range_scatter)
        {
            scatter_force -= scatter_multiplier * dr / dist2;
            dr_nearest_neighbours += dr / dist2;
            n_neighbours++;
        }
        if (dist2 < range_scatter * 2.f)
        {
            average_neighbour_position += dr;
            n_neighbours_group++;
        }
    }

    dr_nearest_neighbours /= n_neighbours;

    if (n_neighbours > 0 && norm2(dr_nearest_neighbours) >= 0.00001f)
    {
        scatter_force += -scatter_multiplier * dr_nearest_neighbours / norm(dr_nearest_neighbours) - m_vel;
    }

    average_neighbour_position /= n_neighbours_group;
    if (n_neighbours_group > 0)
    {
        // cohesion_force =   * average_neighbour_position - m_vel;
    }

    sf::Vector2f align_force = {0, 0};
    if (align_neighbours_count > 0 && norm2(align_direction) >= 0.001f)
    {
        align_force = align_multiplier * align_direction / norm(align_direction) - m_vel;
    }

    auto dr_to_target = m_target_pos - m_pos;
    if (norm(dr_to_target) > 3.f)
    {
        seek_force = seek_multiplier * max_vel * dr_to_target / norm(dr_to_target) - m_vel;
    }

    m_acc += (scatter_force + align_force + seek_force + cohesion_force);
    truncate(m_acc, max_acc);
}


void Enemy::setBehaviour()
{
    auto dice_roll = rand() % 6;
    if (dice_roll < 2)
    {
        m_behaviour = std::make_unique<FollowAndShootAI2>(m_player, this, m_world);
    }
    else if (dice_roll <= 3)
    {
        // m_behaviour = std::make_unique<FollowAndShootAI2>(m_player, this, m_world);
        m_behaviour = std::make_unique<BomberAI>(m_player, this, m_world);
    }
    else
    {
        // m_behaviour = std::make_unique<FollowAndShootAI2>(m_player, this, m_world);
        m_behaviour = std::make_unique<FollowAndShootLasersAI>(m_player, this, m_world);
    }
}


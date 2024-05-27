#include "Entities.h"

#include "CollisionSystem.h"
#include "GridNeighbourSearcher.h"
#include "BehaviourBase.h"
#include "ResourceManager.h"
#include "ResourceHolder.h"

#include "Utils/RandomTools.h"
#include "GameWorld.h"

#include "ResourceManager.h"

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

    truncate(m_acc, max_acc);
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

void Enemy::onCollisionWith(GameObject &obj, CollisionData &c_data)
{
    switch (obj.getType())
    {
    case ObjectType::Bullet:
    {
        auto &bullet = static_cast<Bullet2 &>(obj);
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
        auto& explosion = static_cast<Explosion&>(obj);
        auto dr_to_center = m_pos - obj.getPosition();
        auto dist_to_center = norm(dr_to_center);
        auto impulse_dir = dr_to_center / dist_to_center;

        auto time_factor = explosion.getTimeLeftFraciton();
        auto distance_factor = 1 - dist_to_center / obj.m_collision_shape->getScale().x;
        if (distance_factor > 0)
        {
            m_impulse += time_factor * distance_factor  *impulse_dir * 100.f;
            m_health -= distance_factor*time_factor;
        }
        break;
    }
    case ObjectType::Laser:
        m_health -= 0.1f;
        break;
    }
}

void Enemy::onCreation()
{
    m_neighbour_searcher->insertEntity(*this);
}
void Enemy::onDestruction()
{
    m_neighbour_searcher->removeEntity(m_id);

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
    auto neighbours = m_neighbour_searcher->getNeighboursOfExcept(m_pos, m_boid_radius, m_id);

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

Meteor::Meteor(GameWorld *world, TextureHolder &textures) : GameObject(world, textures, ObjectType::Meteor)
{
    m_collision_shape = std::make_unique<Polygon>();
    m_rigid_body = std::make_unique<RigidBody>();
    initializeRandomMeteor();
}

void Meteor::update(float dt)
{
    truncate(m_vel, max_vel);
    m_pos += m_vel * dt;
}
void Meteor::onCreation()
{
}
void Meteor::onDestruction()
{
}

void Meteor::onCollisionWith(GameObject &obj, CollisionData &c_data)
{
}

void Meteor::draw(sf::RenderTarget &target)
{
    m_collision_shape->draw(target);
}

Meteor::~Meteor() {}

Polygon Meteor::generateRandomConvexPolygon(int n) const
{

    // Generate two lists of random X and Y coordinates
    std::vector<float> xPool(0);
    std::vector<float> yPool(0);

    for (int i = 0; i < n; i++)
    {
        xPool.push_back(randf(-1, 1));
        yPool.push_back(randf(-1, 1));
    }

    // Sort them
    std::sort(xPool.begin(), xPool.end());
    std::sort(yPool.begin(), yPool.end());

    // Isolate the extreme points
    auto minX = xPool.at(0);
    auto maxX = xPool.at(n - 1);
    auto minY = yPool.at(0);
    auto maxY = yPool.at(n - 1);

    // Divide the interior points into two chains & Extract the vector components
    std::vector<float> xVec(0);
    std::vector<float> yVec(0);

    float lastTop = minX, lastBot = minX;

    for (int i = 1; i < n - 1; i++)
    {
        auto x = xPool.at(i);

        if (rand() % 2)
        {
            xVec.push_back(x - lastTop);
            lastTop = x;
        }
        else
        {
            xVec.push_back(lastBot - x);
            lastBot = x;
        }
    }

    xVec.push_back(maxX - lastTop);
    xVec.push_back(lastBot - maxX);

    float lastLeft = minY, lastRight = minY;

    for (int i = 1; i < n - 1; i++)
    {
        auto y = yPool.at(i);

        if (rand() % 2)
        {
            yVec.push_back(y - lastLeft);
            lastLeft = y;
        }
        else
        {
            yVec.push_back(lastRight - y);
            lastRight = y;
        }
    }

    yVec.push_back(maxY - lastLeft);
    yVec.push_back(lastRight - maxY);

    std::random_device rd;
    std::mt19937 g(rd());

    // Randomly pair up the X- and Y-components
    std::shuffle(yVec.begin(), yVec.end(), g);

    // Combine the paired up components into vectors
    std::vector<sf::Vector2f> vec;

    for (int i = 0; i < n; i++)
    {
        vec.emplace_back(xVec.at(i), yVec.at(i));
    }

    // Sort the vectors by angle
    std::sort(vec.begin(), vec.end(), [](const auto &p1, const auto &p2)
              { return std::atan2(p1.y, p1.x) < std::atan2(p2.y, p2.x); });

    // Lay them end-to-end
    float x = 0, y = 0;
    float minPolygonX = 0;
    float minPolygonY = 0;
    std::vector<sf::Vector2f> points;

    for (int i = 0; i < n; i++)
    {
        points.push_back({x, y});

        x += vec.at(i).x;
        y += vec.at(i).y;

        minPolygonX = std::min(minPolygonX, x);
        minPolygonY = std::min(minPolygonY, y);
    }

    // Move the polygon to the original min and max coordinates
    auto xShift = minX - minPolygonX;
    auto yShift = minY - minPolygonY;

    for (int i = 0; i < n; i++)
    {
        auto p = points.at(i);
        points.at(i) += sf::Vector2f{xShift, yShift};
    }
    Polygon p;
    p.points = points;

    return p;
}

void Meteor::initializeRandomMeteor()
{
    auto polygon = generateRandomConvexPolygon(12 + rand() % 3);
    auto radius = randf(5, 20);
    polygon.setScale({radius, radius});
    polygon.setPosition(randomPosInBox());

    m_pos = polygon.getPosition();
    m_angle = polygon.getRotation();

    m_vel = {randf(-6, 6), randf(-6, 6)};

    m_rigid_body->angle_vel = randf(-0.02, 0.02);
    m_rigid_body->mass = radius * radius;
    m_rigid_body->inertia = radius * radius * m_rigid_body->mass;

    m_collision_shape = std::make_unique<Polygon>(polygon);
}

void Enemy::setBehaviour()
{
    m_behaviour = std::make_unique<FollowAndShootAI2>(m_player, this, m_world);
}

Bullet2::Bullet2(GameWorld *world, TextureHolder &textures,
                 PlayerEntity *player)
    : m_player(player), GameObject(world, textures, ObjectType::Bullet)
{
    m_collision_shape = std::make_unique<Polygon>(4);
    m_collision_shape->setScale({1, 1});
}

Bullet2::~Bullet2() {}

void Bullet2::update(float dt)
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

void Bullet2::onCollisionWith(GameObject &obj, CollisionData &c_data)
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

void Bullet2::onCreation()
{
}

void Bullet2::onDestruction()
{
}

void Bullet2::draw(sf::RenderTarget &target)
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

Bomb2::Bomb2(GameWorld *world, TextureHolder &textures,
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
                                              7, 2, m_life_time / 0.016f);
}

Bomb2::~Bomb2() {}

void Bomb2::update(float dt)
{
    m_life_time -= dt;

    m_vel -= 0.05f * m_vel;

    if (m_life_time < 0.f)
    {
        kill();
    }

    m_animation->update();

    m_pos += m_vel * dt;
}

void Bomb2::onCollisionWith(GameObject &obj, CollisionData &c_data)
{
}

void Bomb2::onCreation()
{
}

void Bomb2::onDestruction()
{

    auto meteors = m_neighbour_searcher->findNearestObjects(ObjectType::Meteor, m_pos, m_explosion_radius);
    for (auto p_meteor : meteors)
    {
        p_meteor->kill();
    }

    auto &explosion = m_world->addObject(ObjectType::Explosion);
    explosion.setPosition(m_pos);
}

void Bomb2::draw(sf::RenderTarget &target)
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

Laser2::Laser2(GameWorld *world, TextureHolder &textures,
               Collisions::CollisionSystem &neighbour_searcher)
    : m_neighbour_searcher(&neighbour_searcher), GameObject(world, textures, ObjectType::Laser)
{
    m_collision_shape = std::make_unique<Polygon>(4);

    m_is_bloomy = true;
}

Laser2::~Laser2() {}

void Laser2::update(float dt)
{
    m_life_time -= dt;

    m_length += 3.f;
    if (m_owner)
    {
        m_pos = m_owner->getPosition();
        m_angle = m_owner->getAngle();
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

void Laser2::onCollisionWith(GameObject &obj, CollisionData &c_data)
{
}

void Laser2::onCreation()
{
}

void Laser2::onDestruction()
{
}

void Laser2::draw(sf::RenderTarget &target)
{
    sf::RectangleShape rect;
    rect.setOrigin({0, m_width / 2.f});
    rect.setPosition(m_pos);
    rect.setRotation(m_angle);
    rect.setSize({m_length, m_width});
    rect.setFillColor(sf::Color::White);
    target.draw(rect);
}

Explosion::Explosion(GameWorld *world, TextureHolder &textures)
    : GameObject(world, textures, ObjectType::Explosion)
{
    m_collision_shape = std::make_unique<Polygon>(4);
    m_collision_shape->setScale({2 * m_explosion_radius, 2 * m_explosion_radius});

    auto texture_size = static_cast<sf::Vector2i>(m_textures.get(Textures::ID::Explosion2).getSize());

    m_animation = std::make_unique<Animation>(texture_size,
                                              12, 1, m_life_time / 0.016f, 1, true);
}

Explosion::~Explosion() {}

void Explosion::update(float dt)
{
    m_time += dt;

    if (m_time > m_life_time)
    {
        kill();
    }

    m_animation->update();

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

void Explosion::draw(sf::RenderTarget &target)
{
    sf::RectangleShape rect;
    rect.setTexture(&m_textures.get(Textures::ID::Explosion2));
    rect.setTextureRect(m_animation->getCurrentTextureRect());
    rect.setFillColor(sf::Color(255, 255, 255, 155));

    rect.setOrigin({m_explosion_radius, m_explosion_radius});
    rect.setPosition(m_pos);
    rect.setRotation(m_angle);
    rect.setSize({2 * m_explosion_radius, 2 * m_explosion_radius});
    target.draw(rect);
}

ExplosionAnimation::ExplosionAnimation(GameWorld *world, TextureHolder &textures)
    : GameObject(world, textures, ObjectType::Explosion)
{
    m_collision_shape = std::make_unique<Polygon>(4);
    m_collision_shape->setScale({2 * m_explosion_radius, 2 * m_explosion_radius});

    auto texture_size = static_cast<sf::Vector2i>(m_textures.get(Textures::ID::Explosion).getSize());

    m_animation = std::make_unique<Animation>(texture_size,
                                              4, 4, m_life_time / 0.016f, 1, false);
}

ExplosionAnimation::~ExplosionAnimation() {}

void ExplosionAnimation::update(float dt)
{
    m_life_time -= dt;

    if (m_life_time < 0.f)
    {
        kill();
    }

    m_animation->update();

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

void ExplosionAnimation::draw(sf::RenderTarget &target)
{
    sf::RectangleShape rect;
    rect.setTexture(&m_textures.get(Textures::ID::Explosion));
    rect.setTextureRect(m_animation->getCurrentTextureRect());
    rect.setFillColor(sf::Color(255, 255, 255, 155));

    rect.setOrigin({m_explosion_radius, m_explosion_radius});
    rect.setPosition(m_pos);
    rect.setRotation(m_angle);
    rect.setSize({2 * m_explosion_radius, 2 * m_explosion_radius});
    target.draw(rect);
}

Heart::Heart(GameWorld *world, TextureHolder &textures)
    : GameObject(world, textures, ObjectType::Heart)
{
    m_collision_shape = std::make_unique<Polygon>(4);
    m_collision_shape->setScale({3, 3});

    m_rigid_body = std::make_unique<RigidBody>();
    m_rigid_body->mass = 0.01f;
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
}
void Heart::onCollisionWith(GameObject &obj, CollisionData &c_data)
{
    if (obj.getType() == ObjectType::Player)
    {
        kill();
    }
}

void Heart::draw(sf::RenderTarget &target)
{

    sf::RectangleShape rect;
    rect.setTexture(&m_textures.get(Textures::ID::Heart));
    rect.setOrigin({1.5, 1.5});
    rect.setPosition(m_pos);
    rect.setRotation(dir2angle(m_vel));
    rect.setSize({3., 3.});
    target.draw(rect);
}

SpaceStation::SpaceStation(GameWorld *world, TextureHolder &textures)
    : GameObject(world, textures, ObjectType::SpaceStation)
{
    m_collision_shape = std::make_unique<Polygon>(4);
    m_size = {10, 10};
    m_collision_shape->setScale(m_size);
    m_rigid_body = std::make_unique<RigidBody>();
    m_rigid_body->mass = 5000000.f;
    m_rigid_body->inertia = 50000000.f;
    m_rigid_body->angle_vel = 0;
}

SpaceStation::~SpaceStation() {}

void SpaceStation::update(float dt)
{

    m_time += dt;
    if (m_time > m_spawn_timer)
    {
        m_time = 0.f;
        auto &new_enemy = m_world->addObject(ObjectType::Enemy);

        float rand_angle = randf(-180, 180);

        new_enemy.setPosition(m_pos + 10.f * angle2dir(rand_angle));
        new_enemy.m_vel = 200.f * angle2dir(rand_angle);
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

        auto alpha = 1 - dist_to_center / obj.m_collision_shape->getScale().x;
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

void SpaceStation::draw(sf::RenderTarget &target)
{

    sf::RectangleShape rect;
    rect.setOrigin(m_size / 2.f);
    rect.setPosition(m_pos);
    rect.setRotation(dir2angle(m_vel));
    rect.setTexture(&m_textures.get(Textures::ID::Station));
    rect.setSize(m_size);
    // rect.setFillColor(sf::Color::Red);
    target.draw(rect);

    sf::RectangleShape health_rect;

    float alpha_health = m_health / m_max_health;
    float h_rect_size = m_size.x * alpha_health;

    health_rect.setPosition(m_pos + sf::Vector2f(-m_size.x / 2.f, -m_size.y / 2.f * 3.f));
    health_rect.setFillColor(sf::Color::Red);

    health_rect.setSize({h_rect_size, 1.f});
    target.draw(health_rect);
}
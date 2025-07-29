#include "Enemy.h"

#include "../DrawLayer.h"
#include "../CollisionSystem.h"
#include "../GridNeighbourSearcher.h"
#include "../BehaviourBase.h"
// #include "../ResourceManager.h"
#include "../Utils/RandomTools.h"
#include "../GameWorld.h"
#include "../Animation.h"

#include "Entities.h"
#include "Attacks.h"
#include "Player.h"

Enemy::Enemy(GameWorld *world, TextureHolder &textures,
             Collisions::CollisionSystem *collider, PlayerEntity *player, GameSystems& systems)
    : m_collision_system(collider), m_player(player), m_systems(&systems),
      GameObject(world, textures, ObjectType::Enemy, collider, player)
{
    m_collision_shape = std::make_unique<Polygon>(4);
    m_size = {16, 16};
    m_collision_shape->setScale(m_size / 2.);
    m_target_pos = player->getPosition();
}

SparseGridNeighbourSearcher<utils::Vector2f> Enemy::m_neighbour_searcher = {50.f};

Enemy::~Enemy() {}

void Enemy::update(float dt)
{
    m_neighbour_searcher.move(getPosition(), getId());
    m_behaviour->update(dt);

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

    m_angle = utils::dir2angle(m_vel);
}

void Enemy::onCollisionWith(GameObject &obj, CollisionData &c_data)
{
    switch (obj.getType())
    {
    case ObjectType::Bullet:
    {
        auto &bullet = static_cast<Bullet &>(obj);
        if (bullet.getTime() > 3.5f)
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
    default:
        break;
    }
}

void Enemy::onCreation()
{
    setBehaviour();
    m_neighbour_searcher.insertAt(getPosition(), getPosition(), getId());

    BoidComponent b_comp = {m_pos, m_vel, m_acc, m_boid_radius};
    m_systems->add(b_comp, getId());
}
void Enemy::onDestruction()
{
    m_neighbour_searcher.remove(getId());
    m_systems->remove<BoidComponent>(getId());

    auto &new_explosion = m_world->addObject2<Explosion>();
    new_explosion.removeCollider();
    new_explosion.m_is_expanding = false;
    new_explosion.setPosition(m_pos);
    new_explosion.m_max_explosion_radius = 4.f;
    new_explosion.setType("Explosion2");

    // SoundManager::play(0);
}

void Enemy::draw(LayersHolder &layers)
{

    auto &target = layers.getCanvas("Unit");
    m_sprite.setPosition(m_pos);
    m_sprite.setRotation(glm::radians(m_angle));
    m_sprite.setScale(m_size / 2.f);

    target.drawSprite(m_sprite);

    Sprite booster;
    utils::Vector2f booster_size = {6, 3};

    booster.setTexture(*m_textures->get("BoosterPurple"));
    booster.setScale(booster_size.x, 1.3 * booster_size.y);
    booster.setPosition(m_pos - m_vel / norm(m_vel) * m_sprite.getScale().y);
    booster.setRotation(m_sprite.getRotation());

    target.drawSprite(booster);
}

void Enemy::avoidMeteors()
{

    const auto &r = m_pos;

    auto nearest_meteors =
        m_collision_system->findNearestObjects(ObjectType::Meteor, r, m_boid_radius);

    m_is_avoiding = false;
    m_cm.clear();
    utils::Vector2f avoid_force = {0, 0};
    for (auto *meteor : nearest_meteors)
    {
        auto &meteor_shape = meteor->getCollisionShape();

        auto r_meteor = meteor_shape.getPosition();
        auto radius_meteor = meteor_shape.getScale().x;
        auto dr_to_target = m_target_pos - m_pos;
        dr_to_target /= norm(dr_to_target);

        auto dr_to_meteor = (r_meteor - r) / norm(r - r_meteor);
        utils::Vector2f dr_norm = {dr_to_meteor.y, -dr_to_meteor.x};
        dr_norm /= utils::norm(dr_norm);

        auto dist_to_meteor = dist(r, r_meteor);
        if (dist_to_meteor < 2. * radius_meteor)
        {
            m_is_avoiding = true;
            const auto angle = utils::angleBetween(dr_to_meteor, dr_to_target);
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
    {Multiplier::SCATTER, 2000.f},
    {Multiplier::SEEK, 10.f}};
std::unordered_map<Multiplier, float> Enemy::m_force_ranges = {
    {Multiplier::ALIGN, 20.f},
    {Multiplier::AVOID, 50.f},
    {Multiplier::SCATTER, 30.f},
    {Multiplier::SEEK, 10.f}};

// void Enemy::doBoidSteeringPre(Enemy &e)
// {
// }
// void Enemy::doBoidSteering(Enemy &e1, Enemy &e2)
// {
//     const float scatter_multiplier = Enemy::m_force_multipliers[Multiplier::SCATTER];
//     const float align_multiplier = Enemy::m_force_multipliers[Multiplier::ALIGN];
//     const float seek_multiplier = Enemy::m_force_multipliers[Multiplier::SEEK];

//     const auto range_align = std::pow(Enemy::m_force_ranges[Multiplier::ALIGN], 2);
//     const auto range_scatter = std::pow(Enemy::m_force_ranges[Multiplier::SCATTER], 2);

// }
// void Enemy::applyBoidSteeringForce(Enemy &e)
// {
// }

class BoidSystem
{
    struct BoidComponent
    {
        float radius;
        utils::Vector2f pos;
        utils::Vector2f vel;
        utils::Vector2f boid_force;
    };

    using GridIndT = std::pair<int, int>;
    using GridNodeType = ContiguousColony<BoidComponent, int>;

public:
    BoidSystem()
        : m_neighbour_searcher(50.)
    {
    }

    void insert(BoidComponent &new_comp, int entity_id)
    {
        m_neighbour_searcher.insertAt(new_comp.pos, new_comp, entity_id);
    }

    void remove(int entity_id)
    {
        m_neighbour_searcher.remove(entity_id);
    }

    void preUpdate()
    {
    }
    void update(float dt)
    {
        //! construct neighbour list

        //! calculate forces
    }

    void postUpdate()
    {
        //! update component owners
    }

    ComponentBlock<BoidComponent, 1000> m_data_block;
    std::array<std::vector<BoidComponent>, 1000> neighbour_lists;

private:
    SparseGridNeighbourSearcher<BoidComponent> m_neighbour_searcher;
};

void Enemy::boidSteering()
{
    auto neighbours2 = m_neighbour_searcher.getNeighbourList(getId(), m_pos, 50.);

    // auto neighbours = m_collision_system->findNearestObjects(ObjectType::Enemy, m_pos, 50.);

    utils::Vector2f repulsion_force(0, 0);
    utils::Vector2f push_force(0, 0);
    utils::Vector2f scatter_force(0, 0);
    utils::Vector2f cohesion_force(0, 0);
    utils::Vector2f seek_force(0, 0);
    float n_neighbours = 0;
    float n_neighbours_group = 0;
    utils::Vector2f dr_nearest_neighbours(0, 0);
    utils::Vector2f average_neighbour_position(0, 0);

    utils::Vector2f align_direction = {0, 0};
    int align_neighbours_count = 0;

    const float scatter_multiplier = Enemy::m_force_multipliers[Multiplier::SCATTER];
    const float align_multiplier = Enemy::m_force_multipliers[Multiplier::ALIGN];
    const float seek_multiplier = Enemy::m_force_multipliers[Multiplier::SEEK];

    auto range_align = std::pow(Enemy::m_force_ranges[Multiplier::ALIGN], 2);
    auto range_scatter = std::pow(Enemy::m_force_ranges[Multiplier::SCATTER], 2);

    for (auto [neighbour_pos, id] : neighbours2)
    {
        // if (p_neighbour == this)
        if (getId() == id)
        {
            continue;
        }
        // auto &neighbour_boid =*p_neighbour;
        // if(ind_j == boid_ind){continue;}
        // const auto dr = neighbour_boid.getPosition() - m_pos;
        const auto dr = neighbour_pos - m_pos;
        const auto dist2 = utils::norm2(dr);

        if (dist2 < range_align)
        {
            // align_direction += neighbour_boid.m_vel;
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

    auto dr_to_target = m_target_pos - m_pos;
    if (norm(dr_to_target) > 3.f)
    {
        seek_force = seek_multiplier * max_vel * dr_to_target / norm(dr_to_target) - m_vel;
    }

    utils::Vector2f align_force = {0, 0};
    if (align_neighbours_count > 0 && norm2(align_direction) >= 0.001f)
    {
        align_force = align_multiplier * align_direction / norm(align_direction) - m_vel;
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
        m_sprite.setTexture(*m_textures->get("EnemyShip"));
    }
    else if (dice_roll <= 3)
    {
        m_behaviour = std::make_unique<BomberAI>(m_player, this, m_world);
        m_sprite.setTexture(*m_textures->get("EnemyBomber"));
    }
    else
    {
        m_behaviour = std::make_unique<FollowAndShootLasersAI>(m_player, this, m_world);
        m_sprite.setTexture(*m_textures->get("EnemyLaser"));
    }
}

SpaceStation::SpaceStation(GameWorld *world, TextureHolder &textures, Collisions::CollisionSystem *collider, PlayerEntity *player)
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
        auto &new_enemy = m_world->addObject2<Enemy>();
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

void SpaceStation::draw(LayersHolder &layers)
{

    auto &target = layers.getCanvas("Unit");

    Sprite rect;
    rect.setPosition(m_pos);
    rect.setRotation(dir2angle(m_vel));
    rect.setTexture(*m_textures->get("Station"));
    rect.setScale(m_size.x, m_size.y);
    // rect.setFillColor(sf::Color::Red);
    target.drawSprite(rect);

    // sf::RectangleShape health_rect;

    // float alpha_health = m_health / m_max_health;
    // float h_rect_size = m_size.x * alpha_health;

    // health_rect.setPosition(m_pos + utils::Vector2f(-m_size.x / 2.f, -m_size.y / 2.f * 3.f));
    // health_rect.setFillColor(sf::Color::Red);

    // health_rect.setSize({h_rect_size, 1.f});
    // target.draw(health_rect);
}

Boss::Boss(GameWorld *world, TextureHolder &textures, Collisions::CollisionSystem *collider, PlayerEntity *player)
    : m_player(player),
      GameObject(world, textures, ObjectType::Boss)
{
    m_collision_shape = std::make_unique<Polygon>(4);
    m_collision_shape->setScale(10, 10);
}

Boss::~Boss() {}

void Boss::aiWhenRecharged(float dt)
{
    if (m_state == State::Patroling)
    {
        if (dist(m_pos, m_player->getPosition()) < 3. * m_vision_radius)
        {
            if (rand() % 2 == 0)
            {
                m_state = State::Shooting;
            }
            else
            {
                m_state = State::ShootingLasers;
            }
        }
        else
        {
            m_target_pos = m_player->getPosition();
        }
    }
    else if (m_state == State::Shooting)
    {
        m_shooting_timer += dt;
        if (m_shooting_timer > m_shooting_cooldown)
        {
            m_shooting_timer = 0;
            shootAtPlayer();
            m_state = State::ShootingLasers;
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
            // max_vel = 60.f;
        }
    }
    else if (m_state == State::ShootingLasers)
    {
        m_shooting_timer += dt;
        m_shooting_timer2 += dt;
        if (m_shooting_timer2 > m_lasering_cooldown / 3.)
        {
            m_shooting_timer2 = 0.;
            shootLaserAtPlayer();
        }
        if (m_shooting_timer > m_lasering_cooldown)
        {
            m_shooting_timer = 0;
            shootLasers();
            m_is_recharging = true;
            m_acc *= 0.f;
            m_vel *= 0.f;
        }
    }
}
void Boss::update(float dt)
{
    //! reached target so create new target
    if (dist(m_pos, m_target_pos) < m_vision_radius / 10.f)
    {
        m_target_pos = m_pos + utils::angle2dir(randf(0, 360)) * 30.f;
    }

    if (!m_is_recharging)
    {
        aiWhenRecharged(dt);
    }
    if (m_is_recharging)
    {
        m_max_vel -= 0.09 * m_max_vel;
        if ((m_shooting_timer += dt) > m_recharge_time)
        {
            m_max_vel = m_orig_max_vel;
            m_shooting_timer = 0.;
            m_is_recharging = false;
            m_state = State::Patroling;
        }
    }

    if (m_state != State::Patroling && dist(m_pos, m_player->getPosition()) > 3. * m_vision_radius)
    {
        m_state = State::Patroling;
    }

    auto dr_to_target = m_target_pos - m_pos;
    m_acc = m_max_vel * dr_to_target;

    truncate(m_acc, m_max_vel);
    m_vel += m_acc * dt;

    truncate(m_vel, m_max_vel);
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

    float bullet_angle = utils::dir2angle(m_player->getPosition() - m_pos);
    for (int i = -10; i <= 10; ++i)
    {
        auto angle = bullet_angle + i * (2.f);
        auto &bullet = m_world->addObject2<Bullet>();
        auto dir = utils::angle2dir(angle);
        bullet.setPosition(m_pos + 12.f * dir);
        bullet.setTarget(m_player);
        bullet.m_vel = dir * 100.f;
    }
}
void Boss::throwBombsAtPlayer()
{
    float bullet_angle = utils::dir2angle(m_player->getPosition() - m_pos);

    auto angle = bullet_angle + m_bomb_count * (30.f);
    auto &bomb = m_world->addObject2<Bomb>();
    auto dir = utils::angle2dir(angle);
    bomb.setPosition(m_pos + 12.f * dir);
    bomb.m_vel = dir * 100.f;
    m_bomb_count++;
}
void Boss::shootLasers()
{
    float player_angle = utils::dir2angle(m_player->getPosition() - m_pos);
    for (int i = 0; i < 15; ++i)
    {
        auto angle = player_angle + i / 15.f * 360.f;
        auto &laser = m_world->addObject2<Laser>();
        auto dir = utils::angle2dir(angle);
        laser.setPosition(m_pos + 12.f * dir);
        laser.setAngle(angle);
        laser.setOwner(this);
        laser.m_rotates_with_owner = false;
        laser.m_max_length = 200.;
        laser.m_max_dmg = 0.5;
    }
}
void Boss::shootLaserAtPlayer()
{
    float player_angle = utils::dir2angle(m_player->getPosition() - m_pos);
    for (int i = -1; i <= 1; ++i)
    {
        auto angle = player_angle + i / 3.f * 10.f;
        auto &laser = m_world->addObject2<Laser>();
        auto dir = utils::angle2dir(angle);
        laser.setPosition(m_pos + 12.f * dir);
        laser.setAngle(angle);
        laser.setOwner(this);
        laser.m_rotates_with_owner = false;
        laser.m_life_time = 2.5;
        laser.m_max_length = 250.;
        laser.m_max_dmg = 0.3;
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
        auto mvt = c_data.separation_axis;
        if (dot(mvt, m_vel) < 0.f)
        {
            auto vel_to_meteor = mvt * utils::dot(m_vel, mvt);
            obj.setPosition(obj.getPosition() + vel_to_meteor * 16.6666 / 1000. * 3.0);
            obj.m_vel = vel_to_meteor;
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
            // m_impulse += time_factor * distance_factor * impulse_dir * 100.f;
            m_health -= distance_factor * time_factor * 0.1f;
        }
        break;
    }
    case ObjectType::Laser:
    {
        auto &laser = static_cast<Laser &>(obj);
        if (laser.getOwner() == m_player) //! be dmged only by player
        {
            m_health -= laser.m_max_dmg;
        }
        break;
    }
    }
}

void Boss::onCreation()
{
    m_target_pos = m_pos;
}
void Boss::onDestruction()
{
    auto &new_explosion = m_world->addObject2<Explosion>();
    new_explosion.removeCollider();
    new_explosion.setPosition(m_pos);
    new_explosion.m_explosion_radius = 10.f;
    new_explosion.setType("Explosion2");

    // SoundManager::play(0);
}

void Boss::draw(LayersHolder &layers)
{

    auto &target = layers.getCanvas("Unit");
    Sprite rect;

    rect.setTexture(*m_textures->get("BossShip"));

    m_angle = glm::radians(dir2angle(m_vel));
    rect.setPosition(m_pos);
    rect.setRotation(m_angle);
    rect.setScale(10, 10);

    target.drawSprite(rect);

    Sprite booster;
    utils::Vector2f booster_size = {4, 2};

    booster.setTexture(*m_textures->get("BoosterPurple"));
    booster.setScale(booster_size.x, booster_size.y);
    booster.setPosition(m_pos - m_vel / norm(m_vel) * rect.getScale().y);
    booster.setRotation(rect.getRotation());
    target.drawSprite(booster);

    //!
    if (m_is_recharging)
    {
        utils::Vector2f booster_size = {20, 7.69};
        Sprite booster_rect;
        booster_rect.setTexture(*m_textures->get("Arrow"));
        float booster_ratio = std::min({1.f, 1.f - m_shooting_timer / m_recharge_time});
        booster_rect.setPosition(m_pos.x - booster_size.x / 2.f * (1. - booster_ratio), m_pos.y + 20.);
        booster_rect.setScale(booster_size.x / 2.f * booster_ratio, booster_size.y / 2.f);
        unsigned char factor = 255 * (booster_ratio);
        unsigned char factor2 = 255 * (1. - booster_ratio);
        booster_rect.m_color = ColorByte{255, 0, 0, 255};
        target.drawSprite(booster_rect, "boostBar");
    }
    //! draw health bar
    utils::Vector2f health_bar_size = {30., 5.69};
    Sprite health_bar;
    health_bar.setTexture(*m_textures->get("Arrow"));
    float booster_ratio = std::min({1.f, m_health / m_max_health});
    health_bar.setPosition(m_pos.x - health_bar_size.x / 2.f * (1. - booster_ratio), m_pos.y + 10.);
    health_bar.setScale(health_bar_size.x / 2.f * booster_ratio, health_bar_size.y / 2.f);
    target.drawSprite(health_bar, "healthBar");
}

Turret::Turret(GameWorld *world, TextureHolder &textures, Collisions::CollisionSystem *collider, PlayerEntity *player)
    : p_player(player), GameObject(world, textures, ObjectType::SpaceStation)
{
    m_collision_shape = std::make_unique<Polygon>(4);
    m_size = {10, 10};
    m_collision_shape->setScale(m_size.x, m_size.y);
    m_rigid_body = std::make_unique<RigidBody>();
    m_rigid_body->mass = 5000000.f;
    m_rigid_body->inertia = 50000000.f;
    m_rigid_body->angle_vel = 0;
}

Turret::~Turret() {}

void Turret::update(float dt)
{
    m_time += dt;

    auto dr_to_player = p_player->getPosition() - m_pos;
    float dist_to_player = utils::norm(dr_to_player);
    if (dist_to_player < m_shooting_range)
    {
        if (m_time > m_spawn_timer)
        {
            m_time = 0.f;
            auto &bullet = m_world->addObject2<Bullet>();
            bullet.setBulletType(BulletType::Fire);
            bullet.m_vel = dr_to_player;
            bullet.setSize({2, 2});
            bullet.setPosition(m_pos + dr_to_player / dist_to_player * m_size.x);
        }

        auto my_dir = utils::angle2dir(m_angle);
        auto cos_angle_diff = utils::dot(dr_to_player / dist_to_player, my_dir);
        //! if the angle difference is large enough turn by speed, otherwise set angle to player
        if (cos_angle_diff < 0.999)
        {
            bool clockwise = utils::cross(my_dir, dr_to_player) > 0;
            if(clockwise)
            {
                m_angle += m_turn_speed * dt;
            }else{
                m_angle -= m_turn_speed * dt;
            }
        }
        else
        {
            m_angle = utils::dir2angle(dr_to_player);
        }
    }

    if (m_health < 0.f)
    {
        kill();
    }
}

void Turret::onCollisionWith(GameObject &obj, CollisionData &c_data)
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
        m_health -= 0.1f * alpha;
        break;
    }
    case ObjectType::Laser:
        m_health -= 0.5f;
        break;
    }
}

void Turret::onCreation()
{
}

void Turret::onDestruction()
{
}

void Turret::draw(LayersHolder &layers)
{

    auto &target = layers.getCanvas("Unit");

    Sprite base;
    base.setPosition(m_pos);
    base.setTexture(*m_textures->get("Turrets"));
    base.m_tex_rect = {175, 40, 80, 80};
    base.setScale(m_size.x, m_size.y);
    target.drawSprite(base);

    base.setRotation(utils::to_radains * (m_angle - 90));
    base.m_tex_rect = {285, 197 - 94 - 71, 74, 94};
    target.drawSprite(base);

    // sf::RectangleShape health_rect;

    // float alpha_health = m_health / m_max_health;
    // float h_rect_size = m_size.x * alpha_health;

    // health_rect.setPosition(m_pos + utils::Vector2f(-m_size.x / 2.f, -m_size.y / 2.f * 3.f));
    // health_rect.setFillColor(sf::Color::Red);

    // health_rect.setSize({h_rect_size, 1.f});
    // target.draw(health_rect);
}
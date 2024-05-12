#pragma once

#include "core.h"
#include "Utils/GayVector.h"
#include "Utils/Grid.h"
#include "Utils/RandomTools.h"
#include "Geometry.h"
#include "Polygon.h"
#include "Player.h"
#include "PolygonObstacleManager.h"
#include "ExplosionEffect.h"

struct ProjectileEntity{


    sf::Vector2f pos;
    sf::Vector2f vel = {0,0};
    float orientation = 0.f;
    int damage = 1;

    int lifetime = 0;
    int max_lifetime = 60;
};

struct Bullet 
{

    sf::Vector2f pos;
    sf::Vector2f vel;
    float max_speed = 2.f;
    int dmg = 1;
    float radius = 1.f;
    float orientation = 0.f;

    int lifetime = 0;
    int max_lifetime = 500;
    Player *player = nullptr;
    sf::Vector2f target = {-1, -1};
    int shooter_ent_ind = -1;
};

struct Laser
{

    sf::Vector2f start_position;
    float angle = 0;
    float length = 500.f;
    int time = 0;
    float width = 1.f;
    int shooter_ind = -1;
};

struct Bomb {

    sf::Vector2f pos;
    sf::Vector2f vel;
    float slowing_factor = 0.1f;
    float radius = 2.f;
    float explosion_radius = 25.f;
    int timer = 0;
    int explosion_time = 120;
};

constexpr float MAX_AGENT_SIZE = 5.f;
constexpr int N_BULLET_GRID_X = Geometry::BOX[0] / MAX_AGENT_SIZE + 1;
constexpr int N_BULLET_GRID_Y = Geometry::BOX[1] / MAX_AGENT_SIZE + 1;

class BoidSystem;

struct BulletSystem
{

    GayVector2<Bullet, 10000> bullets;

    BoidSystem *p_boids = nullptr;
    PolygonObstacleManager *p_meteors = nullptr;
    EffectsManager *p_effects = nullptr;

    std::array<std::vector<int>, N_BULLET_GRID_X * N_BULLET_GRID_Y> grid2entities;
    std::unordered_set<int> visited_grids;
    std::unique_ptr<SearchGrid> p_bullet_grid;

    GayVector2<Laser, 100> lasers;
    GayVector2<Bomb, 100> bombs;

    ResourceHolder<sf::Texture, Textures::ID> bomb_textures; 
    std::unique_ptr<AnimatedSpriteEffect> bomb_sprite;

    Player& player;

    BulletSystem(Player& player) : player(player)
    {
        
        bomb_textures.load(Textures::ID::Bomb, "../Resources/bomb.png");
        bomb_sprite = std::make_unique<AnimatedSpriteEffect>(
            bomb_textures.get(Textures::ID::Bomb), sf::Vector2f{0,0}, sf::Vector2f{0,0}, 7, 2
            );

        sf::Vector2f box_size = {Geometry::BOX[0], Geometry::BOX[1]};
        const sf::Vector2i n_cells = {N_BULLET_GRID_X, N_BULLET_GRID_Y};

        p_bullet_grid = std::make_unique<SearchGrid>(n_cells, sf::Vector2f{MAX_AGENT_SIZE, MAX_AGENT_SIZE});
    }

    int findCollidingBoid(int bullet_ind);

    void integrateAndSteer(int bullet_ind);

    void update(float dt = 0.016f);
    void spawnBullet(int shooter_ind, sf::Vector2f at, sf::Vector2f vel, Player *player = nullptr)
    {
        Bullet new_bullet;
        if (shooter_ind == -1)
        {
            new_bullet.lifetime = 10;
        }
        new_bullet.shooter_ent_ind = shooter_ind;
        new_bullet.vel = vel;
        new_bullet.pos = at;
        if (player)
        {
            new_bullet.player = player;
        }
        bullets.insert(new_bullet);
    }

    void spawnBulletNoSeek(int shooter_ind, sf::Vector2f at, sf::Vector2f target)
    {
        Bullet new_bullet;
        new_bullet.shooter_ent_ind = shooter_ind;
        new_bullet.target = target;
        auto dr = new_bullet.target - at;
        new_bullet.vel = dr / norm(dr);
        new_bullet.pos = at;

        bullets.insert(new_bullet);
    }

    void createBomb(int shooter_ind, sf::Vector2f at, sf::Vector2f vel)
    {
        
        Bomb new_bomb;
        new_bomb.pos = at;
        new_bomb.vel = vel;
        

        bombs.insert(new_bomb);

    }

    void createLaser(int shooter_ind, sf::Vector2f at, sf::Vector2f dir, float length)
    {
     
        auto closest_intersection = findClosestIntesection(at, dir, length);
        
        Laser new_laser;
        new_laser.length = dist(at, closest_intersection);
        new_laser.shooter_ind = shooter_ind;
        new_laser.start_position = at;
        auto dr = closest_intersection - at;
        new_laser.angle = std::atan2(dr.y, dr.x) * 180.f / M_PIf;

        lasers.insert(new_laser);
        p_effects->createLaser(at, new_laser.length, new_laser.width, new_laser.angle);
   
   }

    void createLaser(int shooter_ind, sf::Vector2f from, sf::Vector2f to)
    {
        auto dr = to - from;        
        auto l = norm(dr);
        
        createLaser(shooter_ind, from, dr/l, l);
   
    }


    void draw(sf::RenderTarget &window)
    {

        bomb_sprite->setTexture(bomb_textures.get(Textures::ID::Bomb));
        for (const auto& bomb : bombs.data) {

            bomb_sprite->setLifeTime(bomb.explosion_time);
            bomb_sprite->setTime(bomb.timer);    
            bomb_sprite->setPosition(bomb.pos);
            float scale_factor = bomb.radius / bomb_sprite->getTextureRect().width;
            bomb_sprite->setScale({scale_factor, scale_factor});
            window.draw(*bomb_sprite);
        }


        sf::VertexArray bullet_vertices;
        bullet_vertices.setPrimitiveType(sf::Quads);
        bullet_vertices.resize(4 * bullets.size());
        sf::Color color = sf::Color::Red;

        for (int boid_ind = 0; boid_ind < bullets.size(); ++boid_ind)
        {
            const auto &bullet = bullets.data[boid_ind];

            sf::Transform a;
            a.rotate(bullet.orientation);

            bullet_vertices[boid_ind * 4 + 0] = {bullet.pos + a.transformPoint({-0.5, -0.5}), color};
            bullet_vertices[boid_ind * 4 + 1] = {bullet.pos + a.transformPoint({0.5, -0.5}), color};
            bullet_vertices[boid_ind * 4 + 2] = {bullet.pos + a.transformPoint({0.5, 0.5}), color};
            bullet_vertices[boid_ind * 4 + 3] = {bullet.pos + a.transformPoint({-0.5, 0.5}), color};
        }

        window.draw(bullet_vertices);

    }

    void collideWithMeteors(int bullet_ind);

private:

    void explodeBomb(sf::Vector2f center, float radius);

    sf::Vector2f findClosestIntesection(sf::Vector2f at, sf::Vector2f dir, float length){
        sf::Vector2f closest_intersection = at + dir * length;
        float min_dist = 200.f;
        auto inters = p_meteors->collision_tree.rayCast(at, dir, length);
        for (auto ent_idn : inters)
        {
            auto points = p_meteors->meteors.at(ent_idn).getPointsInWorld();
            int next = 1;

            for (int i = 0; i < points.size(); ++i)
            {
                sf::Vector2f r1 = points.at(i);
                sf::Vector2f r2 = points.at(next);

                auto intersection = getIntersection(r1, r2, at, at + dir * 200.f);
                if (intersection.x > 0 & intersection.y > 0)
                {
                    auto new_dist = dist(intersection, at);
                    if (new_dist < min_dist)
                    {
                        closest_intersection = intersection;
                        min_dist = new_dist;
                    }
                }
                next++;
                if (next == points.size())
                {
                    next = 0;
                }
            }
        }
        return closest_intersection;
        
    }



};

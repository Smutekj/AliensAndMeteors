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
    float width = 2.f;
    int shooter_ind = -1;
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

    BulletSystem()
    {
        sf::Vector2f box_size = {Geometry::BOX[0], Geometry::BOX[1]};
        const sf::Vector2i n_cells = {N_BULLET_GRID_X, N_BULLET_GRID_Y};

        p_bullet_grid = std::make_unique<SearchGrid>(n_cells, sf::Vector2f{MAX_AGENT_SIZE, MAX_AGENT_SIZE});
    }

    int findCollidingBoid(int bullet_ind);

    void integrateAndSteer(int bullet_ind);

    void update();
    void spawnBullet(int shooter_ind, sf::Vector2f at, sf::Vector2f vel, Player *player = nullptr)
    {
        Bullet new_bullet;
        if (shooter_ind = -1)
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

        // sf::RectangleShape laser_shape;

        // for(auto& laser: lasers.data){
        //     // auto& laser = lasers.at(laser_ind);
        //     laser_shape.setSize({laser.length, laser.width});
        //     laser_shape.setOrigin({0, laser.width/2.f});
        //     laser_shape.setPosition(laser.start_position);
        //     laser_shape.setRotation(laser.angle);
        //     laser_shape.setFillColor(sf::Color::Yellow);
        //     window.draw(laser_shape);
        // }
    }

    void collideWithMeteors(int bullet_ind);

private:
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

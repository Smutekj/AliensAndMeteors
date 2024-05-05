#pragma once

#include "core.h"
#include "Utils/GayVector.h"
#include "Utils/Grid.h"
#include "Utils/RandomTools.h"
#include "Geometry.h"
#include "Polygon.h"
#include "Player.h"
#include "PolygonObstacleManager.h"


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
    Player* player = nullptr;
    sf::Vector2f target = {-1, -1};
    int shooter_ent_ind = -1;
};

struct Laser{

    sf::Vector2f start_position;
    float angle = 0;
    float length = 500.f;
    int time = 0;
    float width = 5.f;
};

constexpr float MAX_AGENT_SIZE = 5.f;
constexpr int N_BULLET_GRID_X = Geometry::BOX[0] / MAX_AGENT_SIZE + 1;
constexpr int N_BULLET_GRID_Y = Geometry::BOX[1] / MAX_AGENT_SIZE + 1;

class BoidSystem;

struct BulletSystem
{

    GayVector2<Bullet, 10000> bullets;

    BoidSystem *p_boids;
    PolygonObstacleManager* p_meteors;

    std::array<std::vector<int>, N_BULLET_GRID_X * N_BULLET_GRID_Y> grid2entities;
    std::unordered_set<int> visited_grids;
    std::unique_ptr<SearchGrid> p_bullet_grid;

    ObjectPool<Laser, 100> lasers;

    BulletSystem()
    {
        sf::Vector2f box_size = {Geometry::BOX[0], Geometry::BOX[1]};
        const sf::Vector2i n_cells = {N_BULLET_GRID_X, N_BULLET_GRID_Y};

        p_bullet_grid = std::make_unique<SearchGrid>(n_cells, sf::Vector2f{MAX_AGENT_SIZE, MAX_AGENT_SIZE});
    }

    int findCollidingBoid(int bullet_ind); 

    void integrateAndSteer(int bullet_ind);

    void update();
    void spawnBullet(int shooter_ind, sf::Vector2f at, sf::Vector2f vel, Player* player = nullptr){
        Bullet new_bullet;
        new_bullet.shooter_ent_ind = shooter_ind;
        new_bullet.vel = vel;
        new_bullet.pos = at;
        if(player){
            new_bullet.player = player;
        }
        bullets.insert(new_bullet);
    }

    void spawnBulletNoSeek(int shooter_ind, sf::Vector2f at, sf::Vector2f target){
        Bullet new_bullet;
        new_bullet.shooter_ent_ind = shooter_ind;
        new_bullet.target = target;
        auto dr = new_bullet.target - at;
        new_bullet.vel = dr / norm(dr) ;
        new_bullet.pos = at;

        bullets.insert(new_bullet);
    }

    void createLaser(int shooter_ind, sf::Vector2f at, sf::Vector2f dir){
        Laser new_laser;
        new_laser.start_position = at;
        new_laser.angle = std::atan2(dir.y, dir.x)*180.f/M_PIf;
        auto new_int = lasers.addObject(new_laser);

    }   

    void draw(sf::RenderWindow& window){
        
        sf::RectangleShape laser_shape;

        for(auto laser_ind: lasers.active_inds){
            auto& laser = lasers.at(laser_ind);
            laser_shape.setSize({laser.length, laser.width});
            laser_shape.setPosition(laser.start_position);
            laser_shape.setRotation(laser.angle);
            laser_shape.setFillColor(sf::Color::Yellow);
            window.draw(laser_shape);
        }
    }

    void collideWithMeteors(int bullet_ind);
};

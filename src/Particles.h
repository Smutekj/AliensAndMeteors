#pragma once

#include "core.h"

#include "Utils/RandomTools.h"
#include "Utils/ObjectPool.h"

struct Particle
{
    sf::Vector2f pos;
    sf::Vector2f vel;
    sf::Vector2f acc;
    sf::Color color;
    int life_time;
    int time = 0;

    Particle() = default;

    Particle(sf::Vector2f init_pos, sf::Vector2f init_vel, sf::Vector2f acc = {0, 0},
             sf::Color color = sf::Color::Blue, int life_time = 69);
};

class Particles
{

    VectorMap<Particle> particle_pool;

    int spawn_cooldown = 1;
    int spawn_timer = 0;

    sf::Vector2f spawn_pos;

    float particle_size = 0.5;

    float m_vel = 3; 
public:

    sf::Color m_color;
    bool is_active = true;

    Particles(sf::Vector2f spawn_pos, int n_max_particles);
    ~Particles() = default;
    void setSpawnPos(sf::Vector2f pos);
    void update(float dt);
    void draw(sf::RenderTarget &target);

    void setVel(float vel)
    {
        m_vel = vel;
    }
};
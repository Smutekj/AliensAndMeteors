#pragma once

#include <SFML/Graphics/Color.hpp>

#include "Utils/RandomTools.h"
#include "Utils/ObjectPool.h"

namespace sf
{
    class RenderTarget;
}

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

public:
    explicit Particles() = default;
    explicit Particles(int n_max_particles = 100);
    ~Particles() = default;
    void update(float dt);
    virtual void draw(sf::RenderTarget &target);
    void setSpawnPos(sf::Vector2f pos);
    void setVel(float vel);
    void setColor(sf::Color color);
    void setAngleSpread(float min_spread, float max_spread);
    void setRepeat(bool repeats);
    void setSize(float size);


private:
    void integrate(float dt);
    void destroyDeadParticles();
    void createParticle();

protected:
    VectorMap<Particle> m_particle_pool;

    int m_spawn_cooldown = 1;
    int m_spawn_timer = 0;
    sf::Vector2f m_spawn_pos;
    float m_particle_size = 0.5;
    float m_vel = 3;
    sf::Color m_color;

    bool m_repeats = true;
    int n_spawned = 0;

    float m_spread_min = 0;
    float m_spread_max = 360;
};

class ColoredParticles : public Particles
{
};

namespace sf
{
    class Texture;
}

class TexturedParticles : public Particles
{

public:
    explicit TexturedParticles(sf::Texture &texture);
    virtual ~TexturedParticles() = default;

    virtual void draw(sf::RenderTarget &target) override;
    void setTexture(sf::Texture &texture);

private:
    sf::Texture *m_texture = nullptr;
};
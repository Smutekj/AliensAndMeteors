#include "Particles.h"

#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Texture.hpp>

Particle::Particle(sf::Vector2f init_pos, sf::Vector2f init_vel, sf::Vector2f acc,
                   sf::Color color, int life_time)
    : pos(init_pos), vel(init_vel), acc(acc), color(color), life_time(life_time)
{
}

Particles::Particles(int n_max_particles)
    : m_particle_pool(n_max_particles)
{
}

void Particles::setSpawnPos(sf::Vector2f pos)
{
    m_spawn_pos = pos;
}

void Particles::setSize(float size)
{
    m_particle_size = size;
}

void Particles::update(float dt)
{

    m_spawn_timer++;
    if (m_spawn_timer >= m_spawn_cooldown)
    {
        m_spawn_timer = 0;
        
        if (!m_repeats && n_spawned < m_particle_pool.capacity())
        {
            createParticle();
            n_spawned++;
        }
        else if (m_repeats)
        {
            createParticle();
        }
    }

    integrate(dt);
    destroyDeadParticles();
}

void Particles::createParticle()
{
    sf::Vector2f rand_vel = randf(0, m_vel) * angle2dir(randf(m_spread_min, m_spread_max));
    sf::Color color = {m_color.r, m_color.g, m_color.b, 150};
    
    Particle new_particle(m_spawn_pos, rand_vel, {0., 0.}, color);
    m_particle_pool.insert(new_particle);
    n_spawned++;
    
}

    void Particles::destroyDeadParticles()
    {
        auto &particles = m_particle_pool.getData();
        std::vector<int> to_destroy;
        for (int p_ind = 0; p_ind < m_particle_pool.size(); ++p_ind)
        {
            auto &particle = particles[p_ind];
            if (particle.time > particle.life_time)
            {
                to_destroy.push_back(p_ind);
            }
        }
        for (auto part_ind : to_destroy)
        {
            m_particle_pool.removeByDataInd(part_ind);
        }
    }

    void Particles::integrate(float dt)
    {
        auto &particles = m_particle_pool.getData();
        for (int p_ind = 0; p_ind < m_particle_pool.size(); ++p_ind)
        {
            auto &particle = particles[p_ind];
            particle.vel += particle.acc * dt;
            particle.pos += particle.vel * dt;
            auto alpha = (particle.time / (float)particle.life_time);
            particle.color.a = 150 * (1 - alpha);
            particle.time++;
        }
    }

    void Particles::draw(sf::RenderTarget & target)
    {
        auto &particles = m_particle_pool.getData();
        auto n_particles = m_particle_pool.size();

        sf::VertexArray vertices;
        vertices.resize(4 * particles.size());
        vertices.setPrimitiveType(sf::PrimitiveType::Quads);

        for (int p_ind = 0; p_ind < n_particles; ++p_ind)
        {
            auto &particle = particles[p_ind];

            vertices[p_ind * 4 + 0] = sf::Vertex(particle.pos + sf::Vector2f{-m_particle_size / 2, -m_particle_size / 2}, particle.color);
            vertices[p_ind * 4 + 1] = sf::Vertex(particle.pos + sf::Vector2f{m_particle_size / 2, -m_particle_size / 2}, particle.color);
            vertices[p_ind * 4 + 2] = sf::Vertex(particle.pos + sf::Vector2f{m_particle_size / 2, m_particle_size / 2}, particle.color);
            vertices[p_ind * 4 + 3] = sf::Vertex(particle.pos + sf::Vector2f{-m_particle_size / 2, m_particle_size / 2}, particle.color);
        }

        target.draw(vertices);
    }

    void Particles::setVel(float vel)
    {
        m_vel = vel;
    }

    void Particles::setColor(sf::Color color)
    {
        m_color = color;
    }

    TexturedParticles::TexturedParticles(sf::Texture & texture)
        : m_texture(&texture), Particles(20)
    {
        m_particle_size = 3;
    }

    void TexturedParticles::draw(sf::RenderTarget & target)
    {

        auto &particles = m_particle_pool.getData();
        auto n_particles = m_particle_pool.size();

        sf::VertexArray vertices;
        vertices.resize(4 * particles.size());
        vertices.setPrimitiveType(sf::PrimitiveType::Quads);

        sf::Vector2f texture_size = asFloat(m_texture->getSize());

        for (int p_ind = 0; p_ind < n_particles; ++p_ind)
        {
            auto &particle = particles[p_ind];

            vertices[p_ind * 4 + 0].position = particle.pos + sf::Vector2f{-m_particle_size / 2, -m_particle_size / 2};
            vertices[p_ind * 4 + 0].texCoords = {0, 0};
            vertices[p_ind * 4 + 0].color.a = particle.color.a;
            vertices[p_ind * 4 + 1].position = particle.pos + sf::Vector2f{m_particle_size / 2, -m_particle_size / 2};
            vertices[p_ind * 4 + 1].texCoords = {texture_size.x, 0};
            vertices[p_ind * 4 + 1].color.a = particle.color.a;
            vertices[p_ind * 4 + 2].position = particle.pos + sf::Vector2f{m_particle_size / 2, m_particle_size / 2};
            vertices[p_ind * 4 + 2].texCoords = {texture_size.x, texture_size.y};
            vertices[p_ind * 4 + 2].color.a = particle.color.a;
            vertices[p_ind * 4 + 3].position = particle.pos + sf::Vector2f{-m_particle_size / 2, m_particle_size / 2};
            vertices[p_ind * 4 + 3].texCoords = {0, texture_size.y};
            vertices[p_ind * 4 + 3].color.a = particle.color.a;
        }

        sf::RenderStates states;
        states.texture = m_texture;
        target.draw(vertices, states);
    }


   void Particles::setAngleSpread(float min_spread, float max_spread)
    {
        m_spread_max = max_spread;
        m_spread_min = min_spread;
    }
    void Particles::setRepeat(bool repeats)
    {
        m_repeats = repeats;
    }



    void TexturedParticles::setTexture(sf::Texture& texture)
    {
        m_texture = &texture;
    }

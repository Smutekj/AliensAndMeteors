#include "Particles.h"

    Particle::Particle(sf::Vector2f init_pos, sf::Vector2f init_vel, sf::Vector2f acc,
             sf::Color color, int life_time)
        : pos(init_pos), vel(init_vel), acc(acc), color(color), life_time(life_time)
    {
    }

    Particles::Particles(sf::Vector2f spawn_pos, int n_max_particles = 100)
        : particle_pool(n_max_particles),
          spawn_pos(spawn_pos)
    {
    }

    void Particles::setSpawnPos(sf::Vector2f pos)
    {
        spawn_pos = pos;
    }

    void Particles::update(float dt)
    {
        if (!is_active)
        {
            return;
        }

        spawn_timer++;
        if (spawn_timer >= spawn_cooldown)
        {
            spawn_timer = 0;
            sf::Vector2f rand_vel(randf(-3, 3), randf(-3, 3));
            float rand_angle = randf(-45, 45) + randf(-45, 45);
            sf::Color color;
            color.r = 255;
            color.a = 255;
            Particle new_particle(spawn_pos, rand_vel, {0., 0.}, color);
            particle_pool.insert(new_particle);
        }

        auto &particles = particle_pool.data;
        std::vector<int> to_destroy;
        for (int p_ind = 0; p_ind < particle_pool.n_active; ++p_ind)
        {
            auto &particle = particles[p_ind];
            particle.vel += particle.acc * dt;
            particle.pos += particle.vel * dt;
            auto alpha = (particle.time / (float)particle.life_time) ;
            particle.color.r = (1-alpha)*255.;
            particle.color.g = 2*alpha * 255.;
            particle.color.a = (1 - alpha)*255;
            particle.color = {255,255,255, 150};
            if (particle.time++ > particle.life_time)
            {
                to_destroy.push_back(particle_pool.data2entity_ind.at(p_ind));
            }
        }

        for (auto part_ind : to_destroy)
        {
            particle_pool.removeByDataInd(part_ind);
        }
    }

    void Particles::draw(sf::RenderTarget &target)
    {
        auto &particles = particle_pool.data;
        auto n_particles = particle_pool.n_active;

        sf::VertexArray vertices;
        vertices.resize(4 * particles.size());
        vertices.setPrimitiveType(sf::PrimitiveType::Quads);

        for (int p_ind = 0; p_ind < n_particles; ++p_ind)
        {
            auto &particle = particles[p_ind];

            vertices[p_ind * 4 + 0] = sf::Vertex(particle.pos + sf::Vector2f{-particle_size/2, -particle_size/2}, particle.color);
            vertices[p_ind * 4 + 1] = sf::Vertex(particle.pos + sf::Vector2f{particle_size/2, -particle_size/2}, particle.color);
            vertices[p_ind * 4 + 2] = sf::Vertex(particle.pos + sf::Vector2f{particle_size/2, particle_size/2}, particle.color);
            vertices[p_ind * 4 + 3] = sf::Vertex(particle.pos + sf::Vector2f{-particle_size/2, particle_size/2}, particle.color);
        }

        target.draw(vertices);
    }
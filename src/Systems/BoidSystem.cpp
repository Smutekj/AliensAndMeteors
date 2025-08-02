#include "BoidSystem.h"


BoidSystem::BoidSystem(ContiguousColony<BoidComponent, int>& boids)
: m_components(boids), m_neighbour_searcher(50.)
{

}

 void BoidSystem::preUpdate(float dt, EntityRegistryT& entities) 
{
    auto comp_count = m_components.data.size();
    for (std::size_t comp_id = 0; comp_id < comp_count; ++comp_id)
    {
        auto &comp = m_components.data[comp_id];
        //! fetch postition of the owning entity
        comp.pos = entities.at(m_components.data_ind2id.at(comp_id))->getPosition();

        m_neighbour_searcher.insertAt(comp.pos, comp.pos, comp_id);
    }
}

void BoidSystem::postUpdate(float dt, EntityRegistryT& entities) 
{
    auto comp_count = m_components.data.size();
    for (std::size_t comp_id = 0; comp_id < comp_count; ++comp_id)
    {
        auto &comp = m_components.data[comp_id];
        entities.at(m_components.data_ind2id.at(comp_id))->m_vel = comp.vel;
    }
    m_neighbour_searcher.clear();
}   
 void BoidSystem::update(float dt) 
{
    auto comp_count = m_components.data.size();
    for (std::size_t comp_id = 0; comp_id < comp_count; ++comp_id)
    {
        steer(m_components.data[comp_id], comp_id, dt);
        // updater(m_components.data[comp_id], dt);
    }
}

void BoidSystem::steer(BoidComponent &comp, int comp_id, float dt)
{
    auto neighbours2 = m_neighbour_searcher.getNeighbourList(comp_id, comp.pos, comp.boid_radius);

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

    const float scatter_multiplier = 100;//Enemy::m_force_multipliers[Multiplier::SCATTER];
    const float align_multiplier = 1.;//Enemy::m_force_multipliers[Multiplier::ALIGN];
    const float seek_multiplier = 1.;//Enemy::m_force_multipliers[Multiplier::SEEK];

    auto range_align = 100.;//std::pow(Enemy::m_force_ranges[Multiplier::ALIGN], 2);
    auto range_scatter = 100.;//std::pow(Enemy::m_force_ranges[Multiplier::SCATTER], 2);

    for (auto [neighbour_pos, id] : neighbours2)
    {
        // if (p_neighbour == this)

        // if(ind_j == boid_ind){continue;}
        const auto dr = neighbour_pos - comp.pos;
        const auto dist2 = utils::norm2(dr);

        if (dist2 < range_align)
        {
            // align_direction += neighbour_boid.comp.vel;
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
        scatter_force += -scatter_multiplier * dr_nearest_neighbours / norm(dr_nearest_neighbours) - comp.vel;
    }

    average_neighbour_position /= n_neighbours_group;
    if (n_neighbours_group > 0)
    {
        // cohesion_force =   * average_neighbour_position - comp.vel;
    }

    auto dr_to_target = comp.target_pos - comp.pos;
    if (norm(dr_to_target) > 3.f)
    {
        seek_force = seek_multiplier * max_vel * dr_to_target / norm(dr_to_target) - comp.vel;
    }

    utils::Vector2f align_force = {0, 0};
    if (align_neighbours_count > 0 && norm2(align_direction) >= 0.001f)
    {
        align_force = align_multiplier * align_direction / norm(align_direction) - comp.vel;
    }

    comp.vel += dt * (scatter_force + align_force + seek_force + cohesion_force);
    truncate(comp.acc, max_acc);
}

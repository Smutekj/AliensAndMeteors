#pragma once

#include "Utils/ContiguousColony.h"
#include "Vector2.h"
#include "GridNeighbourSearcher.h"

struct BoidComponent
{
    utils::Vector2f pos;
    utils::Vector2f vel;
    utils::Vector2f acc; //! resulting acc;
    float boid_radius = 40.f;
};

struct HealthComponent
{
    float hp;
    float max_hp;
    float hp_regen;
};

struct ShieldComponent
{
    float shield;
    float max_shield;
    float shield_regen;
};

class SystemI
{

public:
    virtual void preUpdate(float dt) = 0;
    virtual void update(float dt) = 0;
    virtual void postUpdate(float dt) = 0;
};

template <class ComponentType>
class System : public SystemI
{
public:
    ComponentType &get(int entity_id)
    {
        int comp_id = m_components.id2data_ind.at(entity_id);
        return m_components.data.at(comp_id);
    }

    std::vector<ComponentType> &getComponents()
    {
        return m_components.data;
    }

    bool has(int entity_id) const
    {
        return m_components.id2data_ind.count(entity_id) > 0;
    }

    void add(ComponentType comp, int entity_id)
    {
        m_components.insert(entity_id, comp);
    }

    void erase(int entity_id)
    {
        m_components.erase(entity_id);
    }

    virtual void preUpdate(float dt) override {}
    virtual void postUpdate(float dt) override {}
    virtual void update(float dt) override
    {
        auto comp_count = m_components.data.size();
        for (std::size_t comp_id = 0; comp_id < comp_count; ++comp_id)
        {
            updater(m_components.data[comp_id], dt);
        }
    }

private:
    ContiguousColony<ComponentType, int> m_components;

public:
    std::function<void(ComponentType &, float)> pre_updater;
    std::function<void(ComponentType &, float)> updater;
    std::function<void(ComponentType &, float)> post_updater;
};

template <>
class System<BoidComponent> : public SystemI
{
    bool has(int entity_id) const
    {
        return m_components.contains(entity_id);
    }

    void add(BoidComponent comp, int entity_id)
    {
        m_components.insert(entity_id, comp);
        m_neighbour_searcher.insertAt(comp.pos, comp, entity_id);
    }

    void remove(int entity_id)
    {
        m_components.erase(entity_id);
        m_neighbour_searcher.remove(entity_id);
    }

    virtual void preUpdate(float dt) override
    {
        auto comp_count = m_components.data.size();
        for (std::size_t comp_id = 0; comp_id < comp_count; ++comp_id)
        {
            auto &comp = m_components.data[comp_id];
            m_neighbour_searcher.move(comp.pos, m_components.data_ind2id.at(comp_id));
        }
    }
    virtual void postUpdate(float dt) override
    {
    }
    virtual void update(float dt) override
    {
        auto comp_count = m_components.data.size();
        for (std::size_t comp_id = 0; comp_id < comp_count; ++comp_id)
        {
            // updater(m_components.data[comp_id], dt);
        }
    }

    void steer(BoidComponent& comp)
    {
        auto neighbours2 = m_neighbour_searcher.getNeighbourList(0, comp, comp.boid_radius);

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

private:
    ContiguousColony<BoidComponent, int> m_components;

    SparseGridNeighbourSearcher<BoidComponent> m_neighbour_searcher;

    std::function<void(BoidComponent &, float)> pre_updater;
    std::function<void(BoidComponent &, float)> updater;
    std::function<void(BoidComponent &, float)> post_updater;
};

template <class... ComponentTypes>
class SystemHolder
{

public:
    template <class ComponentType>
    ComponentType &get(int entity_id)
    {
        std::get<System<ComponentType>>(m_systems).get(entity_id);
    }

    template <class ComponentType>
    bool has(int entity_id) const
    {
        return std::get<System<ComponentType>>(m_systems).has(entity_id);
    }

    template <class ComponentType>
    ComponentType &add(ComponentType comp, int entity_id)
    {
        std::get<System<ComponentType>>(m_systems).add(comp, entity_id);
    }
    template <class ComponentType>
    void remove(int entity_id)
    {
        std::get<System<ComponentType>>(m_systems).remove(entity_id);
    }

    template <class... Components>
    void addEntity(int id, Components... comps)
    {
        (add(comps, id), ...);
    }

    void preUpdate(float dt)
    {
        std::apply([dt](auto &&...system)
                   { ((system.preUpdate(dt)), ...); }, m_systems);
    }
    void update(float dt)
    {
        std::apply([dt](auto &&...system)
                   { ((system.update(dt)), ...); }, m_systems);
    }
    void postUpdate(float dt)
    {
        std::apply([dt](auto &&...system)
                   { ((system.postUpdate(dt)), ...); }, m_systems);
    }

private:
    std::tuple<System<ComponentTypes>...> m_systems;
};

using GameSystems = SystemHolder<BoidComponent, HealthComponent, ShieldComponent>;
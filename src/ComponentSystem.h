#pragma once

#include <unordered_map>
#include <memory>
#include <typeindex>

#include "Utils/ContiguousColony.h"
#include "Vector2.h"
#include "GridNeighbourSearcher.h"

#include "Systems/System.h"

#include "Components.h"

template <class... ComponentTypes>
class ComponentWorld
{

public:
    ComponentWorld(EntityRegistryT &entity_registry)
        : m_entity_registry(entity_registry)
    {
    }

    void registerSystem(std::shared_ptr<SystemI> p_system)
    {
        m_systems2[std::type_index(typeid(*p_system))] = p_system;
    }

    template <class ComponentType>
    ComponentType &get(int entity_id)
    {
        std::get<ComponentHolder<ComponentType>>(m_components).get(entity_id);
    }

    template <class ComponentType>
    bool has(int entity_id) const
    {
        return std::get<ComponentHolder<ComponentType>>(m_components).has(entity_id);
    }

    template <class ComponentType>
    void add(ComponentType comp, int entity_id)
    {
        std::get<ComponentHolder<ComponentType>>(m_components).add(comp, entity_id);
    }

    template <class ComponentType>
    void remove(int entity_id)
    {
        std::get<ComponentHolder<ComponentType>>(m_components).erase(entity_id);
    }
    void removeEntity(int entity_id)
    {
        std::apply([entity_id](auto &&...comp_holder)
                   { (comp_holder.erase(entity_id), ...); }, m_components);
    }

    template <class ComponentType>
    ContiguousColony<ComponentType, int> &getComponents()
    {
        return std::get<ComponentHolder<ComponentType>>(m_components).getComponents();
    }

    template <class... Components>
    void addEntity(int id, Components... comps)
    {
        (add(comps, id), ...);
    }

    void preUpdate(float dt)
    {
        for (auto &[system_id, system] : m_systems2)
        {
            system->preUpdate(dt, m_entity_registry);
        }
    }
    void update(float dt)
    {
        for (auto &[system_id, system] : m_systems2)
        {
            system->update(dt);
        }
    }

    void postUpdate(float dt)
    {
        for (auto &[system_id, system] : m_systems2)
        {
            system->postUpdate(dt, m_entity_registry);
        }
    }

private:
    EntityRegistryT &m_entity_registry;

    std::unordered_map<std::type_index, std::shared_ptr<SystemI>> m_systems2;
    std::tuple<ComponentHolder<ComponentTypes>...> m_components;
};

using GameSystems = ComponentWorld<BoidComponent,
                                   HealthComponent,
                                   ShieldComponent,
                                   AvoidMeteorsComponent,
                                   TargetComponent,
                                   TimedEventComponent>;
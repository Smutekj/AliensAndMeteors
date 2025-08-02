#include "TargetSystem.h"


#include "TargetSystem.h"

TargetSystem::TargetSystem(ContiguousColony<TargetComponent, int> &comps)
    : m_components(comps)
{
}

void TargetSystem::preUpdate(float dt, EntityRegistryT &entities)
{
    auto& entity_ids = m_components.data_ind2id;
    auto comp_count = m_components.data.size();
    for (std::size_t comp_id = 0; comp_id < comp_count; ++comp_id)
    {
        auto &comp = m_components.data[comp_id];

        if(comp.p_target)
        {
            comp.target_pos = comp.p_target->getPosition();
        }
        auto dr = comp.target_pos - entities.at(entity_ids.at(comp_id))->getPosition(); 
        float dist_to_target = utils::norm(dr);
        entities.at(entity_ids.at(comp_id))->m_vel += dt * comp.targetting_strength * dr / dist_to_target;
    }
}

void TargetSystem::postUpdate(float dt, EntityRegistryT &entities)
{

}

void TargetSystem::update(float dt)
{
    auto comp_count = m_components.data.size();
    for (std::size_t comp_id = 0; comp_id < comp_count; ++comp_id)
    {
        auto &comp = m_components.data[comp_id];

        if(comp.p_target)
        {
            comp.target_pos = comp.p_target->getPosition();
        }

        
    }
}

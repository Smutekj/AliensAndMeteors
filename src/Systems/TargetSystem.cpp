#include "TargetSystem.h"


#include "TargetSystem.h"

TargetSystem::TargetSystem(ContiguousColony<TargetComponent, int> &comps, EntityRegistryT& entities)
    : m_components(comps), m_entities(entities)
{
}

void TargetSystem::preUpdate(float dt, EntityRegistryT &entities)
{
    m_entities = entities;
}

void TargetSystem::postUpdate(float dt, EntityRegistryT &entities)
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
        auto vel = entities.at(entity_ids.at(comp_id))->m_vel; 
        float dist_to_target = utils::norm(dr);
        if(dist_to_target > 1.)
        {
            entities.at(entity_ids.at(comp_id))->m_acc += (comp.targetting_strength * dr / dist_to_target - vel);
        }else{
            comp.on_reaching_target();
        }
    }
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


void TargetSystem::draw(Renderer& canvas)
{
    auto comp_count = m_components.data.size();
    for (std::size_t comp_id = 0; comp_id < comp_count; ++comp_id)
    {
        auto &comp = m_components.data[comp_id];

        if(comp.p_target)
        {
            auto pos = m_entities.at(m_components.data_ind2id.at(comp_id))->getPosition(); 
            canvas.drawLineBatched(comp.target_pos, pos, 0.4, {1,0,0,1});
        }
    }
}
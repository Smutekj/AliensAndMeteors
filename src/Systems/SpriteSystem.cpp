#include "SpriteSystem.h"


#include "Renderer.h"
#include "DrawLayer.h"
#include "Particles.h"

SpriteSystem::SpriteSystem(ContiguousColony<SpriteComponent, int> &sprites, LayersHolder& layers)
: m_components(sprites), m_layers(layers)
{
    
}

void SpriteSystem::preUpdate(float dt, EntityRegistryT &entities)
{
    auto &comps = m_components.data;
    auto &ids = m_components.data_ind2id;
    for (std::size_t comp_id = 0; comp_id < comps.size(); ++comp_id)
    {
        auto &comp = comps[comp_id];
        comp.sprite.setPosition(entities.at(ids[comp_id])->getPosition());
        comp.sprite.setRotation(glm::radians(entities.at(ids[comp_id])->getAngle()));
        comp.sprite.setScale(entities.at(ids[comp_id])->getSize()/2.f);
    }
}
void SpriteSystem::postUpdate(float dt, EntityRegistryT &entities)
{
}
void SpriteSystem::update(float dt)
{
    auto &comps = m_components.data;
    auto &ids = m_components.data_ind2id;
    for (std::size_t comp_id = 0; comp_id < comps.size(); ++comp_id)
    {
        auto &comp = comps[comp_id];
        auto& canvas = m_layers.getCanvas(comp.layer_id);
        canvas.drawSprite(comp.sprite, comp.shader_id);
    }

}



ParticleSystem::ParticleSystem(ContiguousColony<ParticleComponent, int> &comps, LayersHolder& layers)
: m_components(comps), m_layers(layers)
{
    
}

void ParticleSystem::preUpdate(float dt, EntityRegistryT &entities)
{
    auto &comps = m_components.data;
    auto &ids = m_components.data_ind2id;
    for (std::size_t comp_id = 0; comp_id < comps.size(); ++comp_id)
    {
        auto &comp = comps[comp_id];
        // comp.sprite.setPosition(entities.at(ids[comp_id])->getPosition());
        // comp.sprite.setRotation(glm::radians(entities.at(ids[comp_id])->getAngle()));
        // comp.sprite.setScale(entities.at(ids[comp_id])->getSize()/2.f);
    }
}
void ParticleSystem::postUpdate(float dt, EntityRegistryT &entities)
{
    auto &comps = m_components.data;
    auto &ids = m_components.data_ind2id;
    for (std::size_t comp_id = 0; comp_id < comps.size(); ++comp_id)
    {
        auto &comp = comps[comp_id];
        auto& canvas = m_layers.getCanvas(comp.layer_id);
        comp.particles->draw(canvas);
    }
}
void ParticleSystem::update(float dt)
{
    auto &comps = m_components.data;
    auto &ids = m_components.data_ind2id;
    for (std::size_t comp_id = 0; comp_id < comps.size(); ++comp_id)
    {
        auto &comp = comps[comp_id];
        comp.particles->update(dt);
    }
}


#pragma once

#include "System.h"

class LayersHolder;


class SpriteSystem : public SystemI
{
public:
    SpriteSystem(ContiguousColony<SpriteComponent, int> &boids, LayersHolder& layers);

    virtual void preUpdate(float dt, EntityRegistryT &entities) override;
    virtual void postUpdate(float dt, EntityRegistryT &entities) override;
    virtual void update(float dt) override;

private:
    //! I could also store all of the sprite data directly in SpriteBatch to avoid copying
    //! But it's not a bottleneck right now so who cares?
    ContiguousColony<SpriteComponent, int> &m_components;
    LayersHolder& m_layers;
};

class ParticleSystem : public SystemI
{
public:
ParticleSystem(ContiguousColony<ParticleComponent, int> &comps, LayersHolder& layers);

    virtual void preUpdate(float dt, EntityRegistryT &entities) override;
    virtual void postUpdate(float dt, EntityRegistryT &entities) override;
    virtual void update(float dt) override;

private:
    //! I could also store all of the sprite data directly in SpriteBatch to avoid copying
    //! But it's not a bottleneck right now so who cares?
    ContiguousColony<ParticleComponent, int> &m_components;
    LayersHolder& m_layers;
};
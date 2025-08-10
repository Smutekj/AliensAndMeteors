#pragma once

#include "System.h"
#include "../Components.h"

#include "Renderer.h"

//! TODO: what happens when a targeted entity dies? 
class TargetSystem : public SystemI
{
public:
    TargetSystem(ContiguousColony<TargetComponent, int> &comps, EntityRegistryT &entities);

    virtual void preUpdate(float dt, EntityRegistryT &entities) override;
    virtual void postUpdate(float dt, EntityRegistryT &entities) override;
    virtual void update(float dt) override;

    void draw(Renderer& canvas);

private:
    EntityRegistryT& m_entities;
    ContiguousColony<TargetComponent, int> &m_components;
};

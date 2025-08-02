#pragma once

#include "System.h"
#include "../Components.h"

class TargetSystem : public SystemI
{
public:
    TargetSystem(ContiguousColony<TargetComponent, int> &comps);

    virtual void preUpdate(float dt, EntityRegistryT &entities) override;
    virtual void postUpdate(float dt, EntityRegistryT &entities) override;
    virtual void update(float dt) override;

private:
    ContiguousColony<TargetComponent, int> &m_components;
};

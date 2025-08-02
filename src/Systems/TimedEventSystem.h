#pragma once

#include "System.h"
#include "../Components.h"

class TimedEventSystem : public SystemI
{
public:
    TimedEventSystem(ContiguousColony<TimedEventComponent, int> &comps);

    virtual void preUpdate(float dt, EntityRegistryT &entities) override;
    virtual void postUpdate(float dt, EntityRegistryT &entities) override;
    virtual void update(float dt) override;

private:
    ContiguousColony<TimedEventComponent, int> &m_components;
};

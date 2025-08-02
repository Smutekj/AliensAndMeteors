#pragma once

#include "System.h"

#include "../Components.h"
#include "../GridNeighbourSearcher.h"



class BoidSystem : public SystemI
{
public:
    BoidSystem(ContiguousColony<BoidComponent, int> &boids);

    virtual void preUpdate(float dt, EntityRegistryT &entities) override;
    virtual void postUpdate(float dt, EntityRegistryT &entities) override;
    virtual void update(float dt) override;

private:
    void steer(BoidComponent &comp, int comp_id, float dt);

private:
    float max_vel = 50.f;
    float max_acc = 5.f;

    ContiguousColony<BoidComponent, int> &m_components;
    SparseGridNeighbourSearcher<utils::Vector2f> m_neighbour_searcher;
};

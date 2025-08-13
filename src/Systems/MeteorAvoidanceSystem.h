#pragma once


#include "System.h"
#include "../ComponentSystem.h"

#include "../Components.h"
#include "../BVH.h"


namespace Collisions
{
    class CollisionSystem;
}

class AvoidanceSystem : public SystemI
{
public:
    AvoidanceSystem(ContiguousColony<AvoidMeteorsComponent, int> &comps,
                    GameSystems& systems,
                    Collisions::CollisionSystem& collision_system);

    virtual void preUpdate(float dt, EntityRegistryT &entities) override;
    virtual void postUpdate(float dt, EntityRegistryT &entities) override;
    virtual void update(float dt) override;

private:
    void avoidMeteors(AvoidMeteorsComponent& comp, float dt);
    void avoidMeteors2(AvoidMeteorsComponent& comp, float dt);

private:

    Collisions::CollisionSystem& m_collision_system;
    ContiguousColony<AvoidMeteorsComponent, int> &m_components;
    
    GameSystems& m_systems;

    Polygon meteor_detector;
};



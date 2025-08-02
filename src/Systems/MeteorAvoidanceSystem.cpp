#include "MeteorAvoidanceSystem.h"

#include "../CollisionSystem.h"
#include "../Polygon.h"
#include "../GameObject.h"

AvoidanceSystem::AvoidanceSystem(ContiguousColony<AvoidMeteorsComponent, int> &comps,
                                 Collisions::CollisionSystem& collision_system)
    : m_components(comps), m_collision_system(collision_system)
{
}
void AvoidanceSystem::preUpdate(float dt, EntityRegistryT &entities)
{
    auto comp_count = m_components.data.size();
    for (std::size_t comp_id = 0; comp_id < comp_count; ++comp_id)
    {
        auto &comp = m_components.data[comp_id];
        //! fetch postition of the owning entity
        comp.pos = entities.at(m_components.data_ind2id.at(comp_id))->getPosition();
    }
}
void AvoidanceSystem::postUpdate(float dt, EntityRegistryT &entities)
{
    auto comp_count = m_components.data.size();
    for (std::size_t comp_id = 0; comp_id < comp_count; ++comp_id)
    {
        auto &comp = m_components.data[comp_id];
        //! fetch postition of the owning entity
        entities.at(m_components.data_ind2id.at(comp_id))->m_vel += comp.vel;
    }
}
void AvoidanceSystem::update(float dt)
{
    auto comp_count = m_components.data.size();
    for (std::size_t comp_id = 0; comp_id < comp_count; ++comp_id)
    {
        avoidMeteors(m_components.data[comp_id],  dt);
    }
}

void AvoidanceSystem::avoidMeteors(AvoidMeteorsComponent &comp, float dt)
{

    const auto &r = comp.pos;

    auto nearest_meteors = m_collision_system.findNearestObjects(ObjectType::Meteor, r, comp.radius);

    utils::Vector2f avoid_force = {0, 0};

    float avoidance_multiplier = 10000.f;

    for (auto *meteor : nearest_meteors)
    {
        auto &meteor_shape = meteor->getCollisionShape();

        auto r_meteor = meteor_shape.getPosition();
        auto radius_meteor = meteor_shape.getScale().x;
        auto dr_to_target = comp.target_pos - comp.pos;
        dr_to_target /= utils::norm(dr_to_target);

        auto dist_to_meteor = utils::dist(r, r_meteor);
        auto dr_to_meteor = (r_meteor - r) / dist_to_meteor;
        utils::Vector2f dr_norm = {dr_to_meteor.y, -dr_to_meteor.x};
        dr_norm /= utils::norm(dr_norm);

        if (dist_to_meteor < 2. * radius_meteor)
        {
            const auto angle = utils::angleBetween(dr_to_meteor, dr_to_target);
            const float sign = 2 * (angle < 0) - 1;

            if (std::abs(angle) < 110)
            {
                avoid_force += sign * dr_norm / (dist_to_meteor - radius_meteor / 2.f);
                avoid_force *= avoidance_multiplier;
            }
        }
    }
    truncate(avoid_force, 500000.f);
    comp.vel += dt * avoid_force;
}
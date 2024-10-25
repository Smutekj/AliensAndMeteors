#pragma once

#include <unordered_map>

#include "../GameObject.h"

class BoidAI2;
class PlayerEntity;
class GameWorld;
class Animation;

namespace Collisions
{
    class CollisionSystem;
}

class GridNeighbourSearcher;


class Enemy : public GameObject
{

public:
    Enemy(GameWorld *world, TextureHolder &textures,
          Collisions::CollisionSystem &collider, GridNeighbourSearcher &m_ns, PlayerEntity *player);
    virtual ~Enemy() override;

    virtual void update(float dt) override;
    virtual void onCreation() override;
    virtual void onDestruction() override;
    virtual void draw(LayersHolder &target) override;
    virtual void onCollisionWith(GameObject &obj, CollisionData &c_data) override;

    void setBehaviour();

public:
    static std::unordered_map<Multiplier, float> m_force_multipliers;
    static std::unordered_map<Multiplier, float> m_force_ranges;

    bool m_deactivated = false;
    float m_deactivated_time = 1.f;

    float max_vel = 40.f;
    float max_acc = 100.f;
    float max_impulse_vel = 40.f;

    float m_health = 5;
    utils::Vector2f m_impulse = {0, 0};
    utils::Vector2f m_target_pos;
    std::vector<utils::Vector2f> m_cm;

private:
    void avoidMeteors();
    void boidSteering();

private:
    float m_boid_radius = 30.f;
    utils::Vector2f m_acc;

    std::unique_ptr<BoidAI2> m_behaviour;
    PlayerEntity *m_player;
    GridNeighbourSearcher *m_neighbour_searcher;
    Collisions::CollisionSystem *m_collision_system;

    bool m_is_avoiding = false;
};


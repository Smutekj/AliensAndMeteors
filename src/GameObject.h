#pragma once

#include <memory>

#include "Polygon.h"
#include "Utils/GayVector.h"
#include "BehaviourBase.h"

enum class ObjectType
{
    Enemy,
    Bullet,

};


class GameObject
{

    std::shared_ptr<Polygon> m_collision_shape;
    bool m_does_physics = true;

    virtual void update() = 0;
    virtual void onDeath() = 0;
    virtual void onSpawn() = 0;

};

class Enemy : public GameObject
{
    Player* p_player;
    std::unique_ptr<BoidAI> p_behaviour;
    CollisionSystem* p_collider; //! this is needed because meteor avoidance 


    virtual void update() override
    {
        p_behaviour->update();
    }
};


class Projectile : public GameObject
{

};

class Laser : public GameObject
{

};

class SeekingMissile : public Projectile
{

};


class BulletMissile : public Projectile
{

};




class CollisionResolver
{

    public:

        void resolve(GameObject& object1, GameObject& object2 )
        {

        }
};

class CollisionSystem
{
    struct CollisionData
    {
        GameObject* p_shape;
        int entity_id = -1;
    };

    ObjectPool<CollisionData, 5000> m_colliders;

    CollisionSystem* p_resolver; 


    void update(){

    }
};

class BoidGrid
{
    std::vector<Boid> boids;
    std::vector<GridInds> boid2grid;
    std::vector<std::vector<int>> boid2neighbour_inds;

    ObjectPool<Boid> m_boids;

    std::unique_ptr<SearchGrid> p_grid;
    std::vector<std::vector<int>> grid2boid_inds;

    void update()
    {

    }

    std::vector<> 

};

class GameWorld
{
    CollisionSystem m_collision_system;
    CollisionResolver

};
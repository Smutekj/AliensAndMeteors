#pragma once

#include "BVH.h"

#include <memory>
#include <vector>
#include <unordered_set>

#include "GameObject.h"
#include "Polygon.h"

namespace Collisions
{

    struct CollisionFeature
    {
        sf::Vector2f best_vertex;
        Edgef edge;
    };

    CollisionData inline calcCollisionData(const std::vector<sf::Vector2f> &points1,
                                           const std::vector<sf::Vector2f> &points2);

    class CollisionSystem
    {

        std::unordered_map<ObjectType, BoundingVolumeTree> m_object_type2tree;
        std::unordered_map<int, std::weak_ptr<GameObject>> m_objects;


        struct pair_hash
        {
            inline std::size_t operator()(const std::pair<int, int> &v) const
            {
                return v.first * 31 + v.second;
            }
        };
        
        std::unordered_set<std::pair<int, int>, pair_hash> m_collided;

    public:
        CollisionSystem();

        void insertObject(std::shared_ptr<GameObject> &p_object);
        void removeObject(GameObject &object);
        void update();
        std::vector<int> findNearestObjectInds(ObjectType type, sf::Vector2f center, float radius);
        std::vector<GameObject *> findNearestObjects(ObjectType type, sf::Vector2f center, float radius);
        sf::Vector2f findClosestIntesection(ObjectType type, sf::Vector2f at, sf::Vector2f dir, float length);
        
        private:
        void narrowPhase(const std::vector<std::pair<int, int>> &colliding_pairs);
        CollisionData getCollisionData(Polygon &pa, Polygon &pb) const;
    };

    CollisionData inline calcCollisionData(const std::vector<sf::Vector2f> &points1,
                                           const std::vector<sf::Vector2f> &points2);
    int inline furthestVertex(sf::Vector2f separation_axis, const std::vector<sf::Vector2f> &points);
    CollisionFeature inline obtainFeatures(const sf::Vector2f axis, const std::vector<sf::Vector2f> &points);
    std::vector<sf::Vector2f> inline clip(sf::Vector2f v1, sf::Vector2f v2, sf::Vector2f n, float overlap);

    std::pair<std::vector<sf::Vector2f>, bool> inline clipEdges(
        CollisionFeature &ref_features,
        CollisionFeature &inc_features,
        sf::Vector2f n);

    void inline bounce(GameObject &obj1, GameObject &obj2, CollisionData c_data);

} //! namespace Collisions

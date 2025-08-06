#pragma once

#include "BVH.h"

#include <memory>
#include <vector>
#include <unordered_set>
#include <variant>

#include <Utils/Vector2.h>
#include "GameObject.h"
#include "Polygon.h"

#include "PostOffice.h"
#include "Systems/System.h"
#include "Renderer.h"

namespace Collisions
{

    template <int N_VERTS>
    struct ConvexPolygon : public Transform
    {
        std::array<utils::Vector2f, N_VERTS> vertices;
    };

    struct Circle : public Transform
    {
    };

    struct Edge
    {
        utils::Vector2f from;
        utils::Vector2f t;
        float l;
        Edge() = default;
        Edge(utils::Vector2f from, utils::Vector2f to) : from(from)
        {
            t = to - from;
            l = norm(t);
            t /= l;
        }
        utils::Vector2f to() const { return from + t * l; }
    };

    struct CollisionFeature
    {
        utils::Vector2f best_vertex;
        Edge edge;
    };

    CollisionData inline calcCollisionData(const std::vector<utils::Vector2f> &points1,
                                           const std::vector<utils::Vector2f> &points2);

    class CollisionSystem : public SystemI
    {

        struct ObjectId
        {
            ObjectType type;
            int id;

            constexpr bool operator==(const ObjectId &other) const
            {
                return type == other.type && id == other.id;
            }
        };

        std::unordered_map<int, std::weak_ptr<GameObject>> m_objects;
        std::unordered_map<ObjectType, BoundingVolumeTree> m_object_type2tree;



        struct pair_hash
        {
            inline std::size_t operator()(const std::pair<int, int> &v) const
            {
                return v.first * 31 + v.second;
            }
        };
        struct pair_hash2
        {
            inline std::size_t operator()(const ObjectId &v) const
            {
                return v.id * static_cast<int>(ObjectType::Count) + static_cast<int>(v.type);
            }
        };
        struct pair_hash3
        {
            inline std::size_t operator()(const std::pair<ObjectId, ObjectId> &v) const
            {
                return v.first.id * static_cast<int>(ObjectType::Count) + static_cast<int>(v.first.type) +
                       v.second.id * static_cast<int>(ObjectType::Count) * 1000 + static_cast<int>(v.second.type) * 595;
            }
        };
        using CollisionCallbackT = std::function<void(GameObject &, GameObject &, CollisionData)>;
        std::unordered_map<std::pair<int, int>, CollisionCallbackT, pair_hash> m_registered_resolvers;

        std::unordered_map<ObjectId, GameObject *, pair_hash2> m_objects2;
        // std::unordered_map<ObjectId, GameObject *, pair_hash2> m_objects3;

        std::unordered_set<std::pair<ObjectId, ObjectId>, pair_hash3> m_collided;
        std::unordered_set<std::pair<int, int>, pair_hash> m_collided2;
        std::unordered_set<std::pair<int, int>, pair_hash> m_exceptions;

    public:
        CollisionSystem(PostOffice &messanger, ContiguousColony<CollisionComponent, int>& comps);

        void insertObject(GameObject &obj);
        void removeObject(GameObject &object);
        
        virtual void preUpdate(float dt, EntityRegistryT &entities) override;
        void update(float dt) override {}
        virtual void postUpdate(float dt, EntityRegistryT &entities) override{}


        void draw(Renderer& canvas);

        void registerResolver(ObjectType type_a, ObjectType type_b, CollisionCallbackT callback = nullptr)
        {
            if(!callback)
            {
                //! let objects deal with collisions themselves by deafult
                callback = [type_a, type_b](GameObject& obj_a, GameObject& obj_b, CollisionData c_data){
                    assert(obj_a.getType() == type_a && obj_b.getType() == type_b);

                    obj_a.onCollisionWith(obj_b, c_data);
                    obj_b.onCollisionWith(obj_a, c_data);
                };
            }

            m_registered_resolvers.insert({{(int)type_a, (int)type_b}, callback});
        }

        std::vector<int> findNearestObjectInds(ObjectType type, utils::Vector2f center, float radius) const;
        std::vector<GameObject *> findNearestObjects(ObjectType type, utils::Vector2f center, float radius) const;
        std::vector<GameObject *> findNearestObjects(ObjectType type, AABB colllision_rect) const;
        std::vector<GameObject *> findNearestObjects(AABB colllision_rect) const;

        utils::Vector2f findClosestIntesection(ObjectType type, utils::Vector2f at, utils::Vector2f dir, float length);

        // template <class EntityType1, class EntityType2>
        // void doCollisions(std::function<void(EntityType1 &, EntityType2 &)> callback,
        //                   ContiguousColony<EntityType1, int> &entities1, ContiguousColony<EntityType2, int> &entities2)
        // {
        //     std::vector<std::pair<int, int>> close_pairs;
        //     if constexpr (std::is_same_v<EntityType1, EntityType2>)
        //     {
        //         assert(type_1 == type_2);
        //         close_pairs = m_object_type2tree.at(type_1).findClosePairsWithin();
        //     }
        //     else
        //     {
        //         close_pairs = m_object_type2tree.at(type_1).findClosePairsWith2(m_object_type2tree.at(type_2));
        //     }

        //     for (auto [ind1, ind2] : close_pairs)
        //     {
        //         auto &obj1 = *m_objects2.at({type_1, ind1});
        //         auto &obj2 = *m_objects2.at({type_2, ind2});
        //         callback(static_cast<EntityType1 &>(obj1), static_cast<EntityType2 &>(obj2));
        //     }
        // }

    private:
        ObjectId getId(GameObject &object) const;
        void narrowPhase(const std::vector<std::pair<ObjectId, ObjectId>> &colliding_pairs);
        void narrowPhase2(const std::vector<std::pair<int, int>> &colliding_pairs,
                          EntityRegistryT &entities,
                          CollisionCallbackT& callback);

        CollisionData getCollisionData(Polygon &pa, Polygon &pb) const;

        PostOffice *p_post_office;

        ContiguousColony<CollisionComponent, int> &m_components;
    };

    CollisionData inline calcCollisionData(const std::vector<utils::Vector2f> &points1,
                                           const std::vector<utils::Vector2f> &points2);
    int inline furthestVertex(utils::Vector2f separation_axis, const std::vector<utils::Vector2f> &points);
    CollisionFeature inline obtainFeatures(const utils::Vector2f axis, const std::vector<utils::Vector2f> &points);
    std::vector<utils::Vector2f> inline clip(utils::Vector2f v1, utils::Vector2f v2, utils::Vector2f n, float overlap);
 
    std::vector<utils::Vector2f> inline clipEdges(
        CollisionFeature &ref_features,
        CollisionFeature &inc_features,
        utils::Vector2f n);

    void inline bounce(GameObject &obj1, GameObject &obj2, CollisionData c_data);

} //! namespace Collisions

// struct CollisionTypePairI
// {

//     // std::
// };

// template <class T1, class T2>
// struct CollisionTypePair : public CollisionTypePairI
// {
//     CollisionTypePair(std::function<void(T1 &, T2 &)> callback)
//         : m_callback(callback)
//     {
//     }

//     std::function<void(T1 &, T2 &)> getCallback()
//     {
//         return m_callback;
//     }

// private:
//     std::function<void(T1 &, T2 &)> m_callback;
// };

// class CollisionResolver
// {
// public:
//     template <class T1, class T2>
//     void registerCallback(std::function<void(T1 &, T2 &)> callback)
//     {
//         m_callbacks[getId<T1, T2>()] = std::make_unique<CollisionPair<T1, T2>>(callback);
//     }

//     template <class T1, class T2>
//     std::function<void(T1 &, T2 &)> getCallback()
//     {
//         return static_cast<CollisionPair<T1, T2>>(*(m_callbacks.at(getId<T1, T2>()))).getCallback();
//     }

// private:
//     using EntityPairId = std::pair<std::type_index, std::type_index>;
//     template <class T1, class T2>
//     EntityPairId getId() const
//     {
//         return {std::type_index(typeid(T1)), std::type_index(typeid(T2))};
//     }

//     std::unordered_map<EntityPairId, std::unique_ptr<CollisionTypePairI>> m_callbacks;
// };

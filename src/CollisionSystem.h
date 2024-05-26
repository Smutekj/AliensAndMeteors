#pragma once

#include "core.h"
#include "BVH.h"

#include <variant>
#include <memory>
#include <vector>
#include <variant>
#include <unordered_set>

#include "GameObject.h"
#include "Polygon.h"

#include "CollisionResolver.h"

namespace Collisions
{

    struct CollisionFeature
    {
        sf::Vector2f best_vertex;
        Edgef edge;
    };

    struct pair_hash
    {
        inline std::size_t operator()(const std::pair<int, int> &v) const
        {
            return v.first * 31 + v.second;
        }
    };

    CollisionData inline calcCollisionData(const std::vector<sf::Vector2f> &points1,
                                           const std::vector<sf::Vector2f> &points2);

    class CollisionSystem
    {

        std::unordered_map<ObjectType, BoundingVolumeTree> m_object_type2tree;
        std::unordered_map<int, GameObject *> m_objects;

        std::unordered_set<std::pair<int, int>, pair_hash> m_collided;

        struct pair_hash
        {
            inline std::size_t operator()(const std::pair<int, int> &v) const
            {
                return v.first * 31 + v.second;
            }
        };

    public:
        CollisionSystem()
        {
            for (int i = 0; i < static_cast<int>(ObjectType::Count); ++i)
            {
                m_object_type2tree[static_cast<ObjectType>(i)] = {};
            }
        }

        void insertObject(GameObject &object);
        void removeObject(GameObject &object);
        void update();
        void narrowPhase(const std::vector<std::pair<int, int>> &colliding_pairs);
        CollisionData getCollisionData(Polygon &pa, Polygon &pb) const;
        std::vector<int> findNearestObjectInds(ObjectType type, sf::Vector2f center, float radius);
        std::vector<GameObject *> findNearestObjects(ObjectType type, sf::Vector2f center, float radius);

        sf::Vector2f findClosestIntesection(ObjectType type, sf::Vector2f at, sf::Vector2f dir, float length);
    };

    CollisionData inline calcCollisionData(const std::vector<sf::Vector2f> &points1,
                                           const std::vector<sf::Vector2f> &points2)
    {
        CollisionData collision_result;

        int next = 1;
        const auto n_points1 = points1.size();
        const auto n_points2 = points2.size();

        Edgef contact_edge;

        float min_overlap = std::numeric_limits<float>::max();
        sf::Vector2f &min_axis = collision_result.separation_axis;
        for (int curr = 0; curr < n_points1; ++curr)
        {

            auto t1 = points1[next] - points1[curr]; //! line perpendicular to current polygon edge
            sf::Vector2f n1 = {t1.y, -t1.x};
            n1 /= norm(n1);
            auto proj1 = projectOnAxis(n1, points1);
            auto proj2 = projectOnAxis(n1, points2);

            if (!overlap1D(proj1, proj2))
            {
                collision_result.minimum_translation = -1;
                return collision_result;
            }
            else
            {
                auto overlap = calcOverlap(proj1, proj2);
                if (overlap < min_overlap)
                {
                    min_overlap = overlap;
                    min_axis = n1;
                }
            }

            next++;
            if (next == n_points1)
            {
                next = 0;
            }
        }
        next = 1;
        for (int curr = 0; curr < n_points2; ++curr)
        {

            auto t1 = points2[next] - points2[curr]; //! line perpendicular to current polygon edge
            sf::Vector2f n1 = {t1.y, -t1.x};
            n1 /= norm(n1);
            auto proj1 = projectOnAxis(n1, points1);
            auto proj2 = projectOnAxis(n1, points2);

            if (!overlap1D(proj1, proj2))
            {
                collision_result.minimum_translation = -1;
                return collision_result;
            }
            else
            {
                auto overlap = calcOverlap(proj1, proj2);
                if (overlap < min_overlap)
                {
                    min_overlap = overlap;
                    min_axis = n1;
                    collision_result.belongs_to_a = false;
                }
            }

            next++;
            if (next == n_points2)
            {
                next = 0;
            }
        }

        collision_result.minimum_translation = min_overlap;
        return collision_result;
    }

    int inline furthestVertex(sf::Vector2f separation_axis, const std::vector<sf::Vector2f> &points)
    {
        float max_dist = -std::numeric_limits<float>::max();
        int index = -1;
        for (int i = 0; i < points.size(); ++i)
        {
            auto dist = dot(points[i], separation_axis);
            if (dist > max_dist)
            {
                index = i;
                max_dist = dist;
            }
        }

        return index;
    }

    CollisionFeature inline obtainFeatures(const sf::Vector2f axis, const std::vector<sf::Vector2f> &points)
    {

        const auto n_points = points.size();
        auto furthest_v_ind1 = furthestVertex(axis, points);

        auto v1 = points[furthest_v_ind1];
        auto v1_next = points[(furthest_v_ind1 + 1) % n_points];
        auto v1_prev = points[(furthest_v_ind1 - 1 + n_points) % n_points];

        auto from_next = v1 - v1_next;
        auto from_prev = v1 - v1_prev;
        from_next /= norm(from_next);
        from_prev /= norm(from_prev);
        Edgef best_edge;
        if (dot(from_prev, axis) <= dot(from_next, axis))
        {
            best_edge = Edgef(v1_prev, v1);
        }
        else
        {
            best_edge = Edgef(v1, v1_next);
        }
        CollisionFeature feature = {v1, best_edge};
        return feature;
    }

    std::vector<sf::Vector2f> inline clip(sf::Vector2f v1, sf::Vector2f v2, sf::Vector2f n, float overlap)
    {

        std::vector<sf::Vector2f> cp;
        float d1 = dot(v1, n) - overlap;
        float d2 = dot(v2, n) - overlap;
        // if either point is past o along n
        // then we can keep the point
        if (d1 >= 0.0)
            cp.push_back(v1);
        if (d2 >= 0.0)
            cp.push_back(v2);
        // finally we need to check if they
        // are on opposing sides so that we can
        // compute the correct point
        if (d1 * d2 < 0.0)
        {
            // if they are on different sides of the
            // offset, d1 and d2 will be a (+) * (-)
            // and will yield a (-) and therefore be
            // less than zero
            // get the vector for the edge we are clipping
            sf::Vector2f e = v2 - v1;
            // compute the location along e
            float u = d1 / (d1 - d2);
            e *= u;
            e += v1;
            // add the point
            cp.push_back(e);
        }
        return cp;
    }

    std::pair<std::vector<sf::Vector2f>, bool> inline clipEdges(CollisionFeature &ref_features, CollisionFeature &inc_features, sf::Vector2f n)
    {

        auto &ref_edge = ref_features.edge;
        auto &inc_edge = inc_features.edge;

        bool flip = false;
        auto wtf_ref = std::abs(dot(ref_edge.t, n));
        auto wtf_inc = std::abs(dot(inc_edge.t, n));
        if (wtf_ref <= wtf_inc)
        {
        }
        else
        {
            std::swap(ref_features, inc_features);
            flip = true;
        }

        sf::Vector2f ref_v = ref_edge.t;

        double o1 = dot(ref_v, ref_edge.from);
        // clip the incident edge by the first
        // vertex of the reference edge
        auto cp = clip(inc_edge.from, inc_edge.to(), ref_v, o1);
        auto cp_new = cp;
        // if we dont have 2 points left then fail
        bool fucked = false;
        if (cp.size() < 2)
        {
            return {};
        }

        // clip whats left of the incident edge by the
        // second vertex of the reference edge
        // but we need to clip in the opposite direction
        // so we flip the direction and offset
        double o2 = dot(ref_v, ref_edge.to());
        cp = clip(cp[0], cp[1], -ref_v, -o2);
        // if we dont have 2 points left then fail
        if (cp.size() < 2)
            return {};

        // get the reference edge normal
        sf::Vector2f refNorm = {-ref_v.y, ref_v.x};
        refNorm /= norm(refNorm);
        // if we had to flip the incident and reference edges
        // then we need to flip the reference edge normal to
        // clip properly
        // if (flip)
        // refNorm *= -1.f;
        // get the largest depth
        double max = dot(refNorm, ref_features.best_vertex);
        // make sure the final points are not past this maximum

        std::vector<float> depths(2);
        depths[0] = dot(refNorm, cp.at(0)) - max;
        depths[1] = dot(refNorm, cp.at(1)) - max;
        // if (depths[0] < 0.0f && depths[1] < 0.f){
        //   return {};
        // }
        if (depths[0] < 0.0f)
        {
            cp.erase(cp.begin());
        }
        if (depths[1] < 0.0f)
        {
            cp.pop_back();
        }
        // return the valid points
        return {cp, flip};
    }

} //! namespace Collisions

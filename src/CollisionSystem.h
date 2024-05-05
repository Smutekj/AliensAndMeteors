#pragma once

#include "core.h"
#include "BVH.h"

#include <variant>
#include <memory>
#include <vector>

namespace Collisions
{

    struct Collidable
    {
        AABB bounding_rect;
    };

    struct Circle : Collidable
    {
        float radius;
        sf::Vector2f r_center;
    };

    struct ConvexShape : Collidable
    {
        std::vector<sf::Vector2f> points;
    };

    class CollisionSystem
    {

        std::vector<std::unique_ptr<Collidable>> collidables_;


        std::vector<std::variant<Circle, ConvexShape>> wtfs;


        BoundingVolumeTree collision_tree;

        void addCollidable(Collidable* new_shape, int entity_ind){

            
            collision_tree.addRect(new_shape->bounding_rect, entity_ind);
        }

        void remove(int entity_ind){
            collision_tree.removeObject(entity_ind);
        }

        std::vector<std::pair<int, int>> findCollisionPairs(){

            // std::visit

            for(auto& collidable : collidables_){
                auto potential_collisions = collision_tree.findIntersectingLeaves(collidable->bounding_rect);
                
            }

        }


        
    };

}




struct CollisionFinder
{

    CollisionData operator()(const Polygon &p1, const Polygon &p2)
    {

        const auto &points1 = p1.getPointsInWorld();
        const auto &points2 = p2.getPointsInWorld();
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

    
    CollisionData operator()(const Sphere &s1, const Sphere &s2)
    {

        CollisionData cd;
        auto dr_12 = s2.r_center - s1.r_center;
        auto dist = norm(dr_12);
        cd.minimum_translation = dist - (s1.radius+s2.radius) ;
        cd.separation_axis = dr_12/dist;
        return cd;
    }

    CollisionData operator()(const Polygon &p1, const Sphere &s2)
    {

        CollisionData collision_result;

        const auto &points_world = p1.getPointsInWorld();

    int next = 1;
    const auto n_points1 = points_world.size();

    float min_overlap = std::numeric_limits<float>::max();
    sf::Vector2f min_axis;
    for (int curr = 0; curr < n_points1; ++curr)
    {
      auto t1 = points_world.at(next) - points_world[curr]; //! line perpendicular to current polygon edge
      sf::Vector2f n1 = {t1.y, -t1.x};
      n1 /= norm(n1);
      auto proj1 = projectOnAxis(n1, points_world);
      float proj_sphere = dot(n1, s2.r_center);
      Projection1D proj2(proj_sphere - s2.radius, proj_sphere + s2.radius);

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
          collision_result.separation_axis = min_axis;
        }
      }
      next++;
      if (next == n_points1)
      {
        next = 0;
      }
    }

        collision_result.minimum_translation = min_overlap;
        return collision_result;
    }

    CollisionData operator()(const Sphere &s1, const Polygon &p2)
    {
// 
        // return this->(p2, s1);
        std::cout << "rect and: sphere\n";
    }
};

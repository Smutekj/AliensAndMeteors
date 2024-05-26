#include "CollisionSystem.h"

#include "GameObject.h"

namespace Collisions
{

    void CollisionSystem::insertObject(GameObject &object)
    {

        auto bounding_rect = object.getCollisionShape().getBoundingRect().inflate(1.2f);
        m_object_type2tree[object.getType()].addRect(bounding_rect, object.getId());

        assert(m_objects.count(object.getId()) == 0);
        m_objects[object.getId()] = &object;
    }

    void CollisionSystem::removeObject(GameObject &object)
    {
        m_object_type2tree.at(object.getType()).removeObject(object.getId());

        assert(m_objects.count(object.getId()) > 0);

        m_objects.erase(object.getId());
    }

    void CollisionSystem::update()
    {

        for (auto &[ind, entity] : m_objects)
        {
            auto type = entity->getType();
            auto &tree = m_object_type2tree.at(type);
            auto fitting_rect = entity->m_collision_shape->getBoundingRect();
            auto big_bounding_rect = tree.getObjectRect(ind);

            //! if object moved in a way that rect in the collision tree does not fully contain it
            if (makeUnion(fitting_rect, big_bounding_rect).volume() > big_bounding_rect.volume())
            {
                tree.removeObject(ind);
                tree.addRect(fitting_rect.inflate(1.2f), ind);
            }
        }

        for (int i = 0; i < static_cast<int>(ObjectType::Count); ++i)
        {

            auto &tree_i = m_object_type2tree.at(static_cast<ObjectType>(i));
            for (int j = i; j < static_cast<int>(ObjectType::Count); ++j)
            { //! for all pairs of object trees;

                auto &tree_j = m_object_type2tree.at(static_cast<ObjectType>(j));

                auto close_pairs = tree_i.findClosePairsWith(tree_j);

                narrowPhase(close_pairs);
            }
        }
        m_collided.clear();
    }

    void CollisionSystem::narrowPhase(const std::vector<std::pair<int, int>> &colliding_pairs)
    {
        for (auto [i1, i2] : colliding_pairs)
        {

            auto obj1 = m_objects.at(i1);
            auto obj2 = m_objects.at(i2);

            if (m_collided.count({i1, i2}) > 0)
            {
                continue;
            }
            m_collided.insert({i1, i2});

            auto collision_data = getCollisionData(obj1->getCollisionShape(), obj2->getCollisionShape());
            if (collision_data.minimum_translation > 0) //! there is a collision
            {
                CollisionResolver resolver(collision_data);
                if (obj1->m_rigid_body && obj2->m_rigid_body) //! if both objects have rigid bodies we do physics
                {
                    resolver.bounce(*obj1, *obj2);
                }

                obj1->onCollisionWith(*obj2, collision_data);
                obj2->onCollisionWith(*obj1, collision_data);
            }
        }
    }

    CollisionData CollisionSystem::getCollisionData(Polygon &pa, Polygon &pb) const
    {
        auto points_a = pa.getPointsInWorld();
        auto points_b = pb.getPointsInWorld();
        auto c_data = calcCollisionData(points_a, points_b);

        if (c_data.minimum_translation < 0.f)
        {
            return c_data; //! there is no collision so we don't need to extract manifold
        }
        auto center_a = pa.getPosition();
        auto center_b = pb.getPosition();
        //! make separation axis point always from a to b
        auto are_flipped = dot((center_a - center_b), c_data.separation_axis) > 0;
        if (are_flipped)
        {
            c_data.separation_axis *= -1.f;
        }

        auto col_feats1 = obtainFeatures(c_data.separation_axis, points_a);
        auto col_feats2 = obtainFeatures(-c_data.separation_axis, points_b);

        auto [clipped_edge, flipped] = clipEdges(col_feats1, col_feats2, c_data.separation_axis);
        if (clipped_edge.size() == 0) //! clipping failed so we don't do collision
        {
            c_data.minimum_translation = -1.f;
            return c_data;
        }
        for (auto ce : clipped_edge)
        {
            c_data.contact_point += ce;
        }
        c_data.contact_point /= (float)clipped_edge.size();

        return c_data;
    }

    std::vector<int> CollisionSystem::findNearestObjectInds(ObjectType type, sf::Vector2f center, float radius)
    {
        auto &tree = m_object_type2tree.at(type);

        AABB collision_rect({center - sf::Vector2f{radius, radius}, center + sf::Vector2f{radius, radius}});
        return tree.findIntersectingLeaves(collision_rect);
    }

    std::vector<GameObject *> CollisionSystem::findNearestObjects(ObjectType type, sf::Vector2f center, float radius)
    {
        auto &tree = m_object_type2tree.at(type);

        AABB collision_rect({center - sf::Vector2f{radius, radius}, center + sf::Vector2f{radius, radius}});
        auto nearest_inds = tree.findIntersectingLeaves(collision_rect);
        std::vector<GameObject *> objects;
        for (auto ind : nearest_inds)
        {
            auto mvt = m_objects.at(ind)->getCollisionShape().getMVTOfSphere(center, radius);
            if (norm2(mvt) > 0.001f)
            {
                objects.push_back(m_objects.at(ind));
            }
        }
        return objects;
    }

    sf::Vector2f CollisionSystem::findClosestIntesection(ObjectType type, sf::Vector2f at, sf::Vector2f dir, float length)
    {
        sf::Vector2f closest_intersection = at + dir * length;
        float min_dist = 200.f;
        auto inters = m_object_type2tree.at(type).rayCast(at, dir, length);
        for (auto ent_ind : inters)
        {
            auto points = m_objects.at(ent_ind)->getCollisionShape().getPointsInWorld();

            int next = 1;
            for (int i = 0; i < points.size(); ++i)
            {
                sf::Vector2f r1 = points.at(i);
                sf::Vector2f r2 = points.at(next);

                auto intersection = getIntersection(r1, r2, at, at + dir * length);
                if (intersection.x > 0 & intersection.y > 0)
                {
                    auto new_dist = dist(intersection, at);
                    if (new_dist < min_dist)
                    {
                        closest_intersection = intersection;
                        min_dist = new_dist;
                    }
                }
                next++;
                if (next == points.size())
                {
                    next = 0;
                }
            }
        }
        return closest_intersection;
    }

} //! namespace collisions
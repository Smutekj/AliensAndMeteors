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
                if (obj1->m_rigid_body && obj2->m_rigid_body) //! if both objects have rigid bodies we do physics
                {
                    bounce(*obj1, *obj2, collision_data);
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


    void inline bounce(GameObject &obj1, GameObject &obj2, CollisionData c_data)
    {
        auto inertia1 = obj1.m_rigid_body->inertia;
        auto inertia2 = obj2.m_rigid_body->inertia;
        auto &angle_vel1 = obj1.m_rigid_body->angle_vel;
        auto &angle_vel2 = obj2.m_rigid_body->angle_vel;
        auto &mass1 = obj1.m_rigid_body->mass;
        auto &mass2 = obj2.m_rigid_body->mass;

        auto n = c_data.separation_axis;
        if (obj1.m_rigid_body->mass < obj2.m_rigid_body->mass)
        {
            obj1.move(-c_data.separation_axis * c_data.minimum_translation);
        }
        else
        {
            obj2.move(c_data.separation_axis * c_data.minimum_translation);
        }

        auto cont_point = c_data.contact_point;

        auto v_rel = obj1.m_vel - obj2.m_vel;
        auto v_reln = dot(v_rel, n);

        float e = 1;
        float u_ab = 1. / mass1 + 1. / mass2;

        auto r_cont_coma = cont_point - obj1.getPosition();
        auto r_cont_comb = cont_point - obj2.getPosition();

        sf::Vector2f r_cont_coma_perp = {r_cont_coma.y, -r_cont_coma.x};
        sf::Vector2f r_cont_comb_perp = {r_cont_comb.y, -r_cont_comb.x};

        float ran = dot(r_cont_coma_perp, n);
        float rbn = dot(r_cont_comb_perp, n);

        float u_ab_rot = ran * ran / inertia1 + rbn * rbn / inertia2;

        float j_factor = -(1 + e) * v_reln / (u_ab + u_ab_rot);

        angle_vel1 += ran * j_factor / inertia1;
        angle_vel2 -= rbn * j_factor / inertia2;
        obj1.m_vel += j_factor / mass1 * n;
        obj2.m_vel -= j_factor / mass2 * n;
    }

} //! namespace collisions
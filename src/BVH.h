#pragma once

#include "core.h"
#include <set>
#include <unordered_map>
#include <queue>

bool inline intersects(const AABB &r1, const AABB &r2)
{
    bool intersects_x = r1.r_min.x <= r2.r_max.x && r1.r_max.x >= r2.r_min.x;
    bool intersects_y = r1.r_min.y <= r2.r_max.y && r1.r_max.y >= r2.r_min.y;
    return intersects_x && intersects_y;
}

struct BVHNode
{
    AABB rect;
    int child_index_1 = -1;
    int child_index_2 = -1;
    int parent_index = -1;
    int object_index = -1;
    int height = 0;

    bool isLeaf() const
    {
        return child_index_1 == -1 && child_index_2 == -1;
    }
};

struct RayCastData
{
    int entity_ind;
    sf::Vector2f hit_point;
    sf::Vector2f hit_normal;
};

class BoundingVolumeTree
{
    std::vector<BVHNode> nodes;
    std::unordered_map<int, int> object2node_indices; //! mapping from objects to leaves

    std::set<int> free_indices;
    int root_ind = -1;

public:
    const BVHNode &getNode(int node_index) const;

    void addRect(AABB rect, int object_index);

    void removeObject(int object_index);

    std::vector<int> findIntersectingLeaves(AABB rect);

    // void draw(sf::RenderWindow &window);

    void clear()
    {
        object2node_indices.clear();
        for (int i = 0; i < nodes.size(); ++i)
        {
            free_indices.insert(i);
        }
    }

    std::vector<int> rayCast(sf::Vector2f from, sf::Vector2f dir, float length)
    {

        sf::Vector2f to = from + dir * length;
        std::vector<int> intersections;

        std::queue<int> to_visit;
        to_visit.push(root_ind);
        while (!to_visit.empty())
        {

            int current_ind = to_visit.front();
            to_visit.pop();
            auto &current = nodes.at(current_ind);

            if (intersectsLine(from, to, current.rect))
            {
                if (!current.isLeaf())
                {
                    to_visit.push(current.child_index_1);
                    to_visit.push(current.child_index_2);
                }
                else
                {
                    intersections.push_back(current.object_index);
                }
            }
        }

        return intersections;
    }

private:
    bool intersectsLine(sf::Vector2f from, sf::Vector2f to, AABB rect);
    bool isLeaf(int node_index) const;
    int balance(int index);
    void removeLeaf(int leaf_index);
    int findBestSibling(const AABB &new_rect);
    int findBestSiblingGreedy(const AABB &new_rect);
    bool containsCycle() const;
    bool isConsistent() const;
    int calcMaxDepth() const;
    void moveNodeUp(int going_up_index);
};
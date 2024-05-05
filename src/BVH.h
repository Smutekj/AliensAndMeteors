#pragma once

#include "core.h"
#include <set>
#include <unordered_map>

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

private:
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
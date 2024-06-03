#pragma once

#include <set>
#include <unordered_map>
#include <vector>

#include "AABB.h"

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


//! Based on this presentation: https://box2d.org/files/ErinCatto_DynamicBVH_Full.pdf
class BoundingVolumeTree
{

public:
    const BVHNode &getNode(int node_index) const;
    void addRect(AABB rect, int object_index);
    void removeObject(int object_index);
    const std::unordered_map<int, int>  &getObjects() const;
    std::vector<std::pair<int, int>> findClosePairsWith(BoundingVolumeTree &tree);
    std::vector<int> findIntersectingLeaves(AABB rect);
    void clear();
    const AABB &getObjectRect(int object_ind) const;
    std::vector<int> rayCast(sf::Vector2f from, sf::Vector2f dir, float length);

private:
    int maxBalanceFactor() const;
    bool intersectsLine(sf::Vector2f from, sf::Vector2f to, AABB rect);
    bool isLeaf(int node_index) const;
    int balance(int index);
    void removeLeaf(int leaf_index);
    int findBestSibling(const AABB &new_rect);
    int findBestSiblingGreedy(const AABB &new_rect);
    int findBestSiblingExhaustive(const AABB& new_rect);
    bool containsCycle() const;
    bool isConsistent() const;
    int calcMaxDepth() const;
    void moveNodeUp(int going_up_index);
    void refitFrom(int node_index);

private:
    std::vector<BVHNode> m_nodes;
    std::unordered_map<int, int> m_object2node_indices; //! mapping from objects to leaves
    std::set<int> m_free_indices;                       //! holds node indices that can be used whe inserting new rect
    int m_root_ind = -1;

};

bool inline intersects(const AABB &r1, const AABB &r2)
{
    bool intersects_x = r1.r_min.x <= r2.r_max.x && r1.r_max.x >= r2.r_min.x;
    bool intersects_y = r1.r_min.y <= r2.r_max.y && r1.r_max.y >= r2.r_min.y;
    return intersects_x && intersects_y;
}


#pragma once

#include <set>
#include <unordered_map>
#include <queue>

#include "core.h"

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
    std::set<int> free_indices;     //! holds node indices that can be used whe inserting new rect
    int root_ind = -1;

public:
    const BVHNode &getNode(int node_index) const;

    void addRect(AABB rect, int object_index);
    
    void removeObject(int object_index);
    
    const auto& getObjects()const
    {
        return object2node_indices;
    }
    
    std::vector<std::pair<int, int>> findClosePairsWith(BoundingVolumeTree& tree)
    {
        std::vector<std::pair<int, int>> close_pairs;

        for(auto& [object_ind, node_ind] : object2node_indices)
        {
            auto nearest_objects_inds = tree.findIntersectingLeaves(nodes.at(node_ind).rect);
            for(auto i : nearest_objects_inds)
            {
                if(i == object_ind){continue;}
                close_pairs.push_back({std::min(object_ind, i), std::max(object_ind, i)});
            }
        }
        return close_pairs;
    }   

    std::vector<int> findIntersectingLeaves(AABB rect);
    
    void clear();
    
    const AABB& getObjectRect(int object_ind) const
    {
        return nodes.at(object2node_indices.at(object_ind)).rect;
    }

    std::vector<int> rayCast(sf::Vector2f from, sf::Vector2f dir, float length);

private:
    int maxBalanceFactor()const;
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

bool inline intersects(const AABB &r1, const AABB &r2)
{
    bool intersects_x = r1.r_min.x <= r2.r_max.x && r1.r_max.x >= r2.r_min.x;
    bool intersects_y = r1.r_min.y <= r2.r_max.y && r1.r_max.y >= r2.r_min.y;
    return intersects_x && intersects_y;
}



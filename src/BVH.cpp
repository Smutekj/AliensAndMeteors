#include "BVH.h"

#include "Polygon.h"

#include <stack>
#include <queue>
#include <iostream>

const BVHNode &BoundingVolumeTree::getNode(int node_index) const
{
    return m_nodes.at(node_index);
}

//! \brief adds new leaf node holding object with index \p object_index
//! \brief which is bound by rectangle \p rect
//! \brief the object with that index must not exist in the tree!
//! \brief finds suitable place for the new node so as to minimize tree volume increase
//! \brief (unless tree is empty) creates 2 new nodes (one leaf for the object and one internal)
//! \brief in the new tree each internal node has exactly two children!
void BoundingVolumeTree::addRect(AABB rect, int object_index)
{
    assert(m_object2node_indices.count(object_index) == 0);

    //! we ran out of free node spots, so we increase node spots
    if (m_free_indices.empty())
    {
        for (int i = m_nodes.size(); i < 2 * m_nodes.size() + 1; ++i)
        {
            m_free_indices.insert(i);
        }
        m_nodes.resize(2 * m_nodes.size() + 1);
    }

    int new_internal = *m_free_indices.begin();
    m_free_indices.erase(m_free_indices.begin());

    if (m_object2node_indices.empty()) //! tree is empty
    {
        m_root_ind = 0;
        m_object2node_indices[object_index] = m_root_ind;
        m_nodes[new_internal] = {rect, -1, -1, -1, object_index};
        return;
    }

    assert(!containsCycle());
    assert(isConsistent());
    assert(m_object2node_indices.count(object_index) == 0);

    int new_leaf = *m_free_indices.begin();
    m_free_indices.erase(m_free_indices.begin());
    m_object2node_indices[object_index] = new_leaf;

    //! find best sibling
    int best_index = findBestSibling(rect);

    int old_parent = m_nodes.at(best_index).parent_index;
    m_nodes.at(new_internal).height = m_nodes.at(best_index).height + 1;
    m_nodes.at(new_internal).child_index_1 = best_index;
    m_nodes.at(new_internal).child_index_2 = new_leaf;
    m_nodes.at(new_internal).parent_index = old_parent;

    //! fix old parents children
    if (best_index != m_root_ind)
    {
        best_index == m_nodes.at(old_parent).child_index_1 ? m_nodes.at(old_parent).child_index_1 = new_internal : m_nodes.at(old_parent).child_index_2 = new_internal;
    }
    else
    { //! best sibling was a root
        m_root_ind = new_internal;
    }

    m_nodes.at(best_index).parent_index = new_internal;
    m_nodes.at(new_leaf).parent_index = new_internal;
    m_nodes.at(new_leaf).rect = rect;
    m_nodes.at(new_leaf).object_index = object_index;
    m_nodes.at(new_internal).rect = makeUnion(m_nodes.at(best_index).rect, rect);

    //! refit bounding volumes
    refitFrom(m_nodes.at(new_leaf).parent_index);

    assert(!containsCycle());
    assert(isConsistent());
}

//! \brief removes object with \p object_index from the tree
//! \brief the object must be present in the tree!
void BoundingVolumeTree::removeObject(int object_index)
{
    assert(m_object2node_indices.count(object_index) > 0);

    auto leaf_index = m_object2node_indices.at(object_index);

    removeLeaf(leaf_index);
    m_object2node_indices.erase(object_index);
    assert(!containsCycle());
    assert(isConsistent());
}

//! \brief removes leaf with node index \p leaf index
//! \brief does necessary bookeeping and refits the bounding volumes
void BoundingVolumeTree::removeLeaf(int leaf_index)
{

    assert(m_nodes.at(leaf_index).isLeaf());
    if (leaf_index == m_root_ind) //! there is just one leaf thus it is root
    {
        m_free_indices.insert(leaf_index);
        return;
    }

    const auto &removed_leaf_node = m_nodes.at(leaf_index);
    const auto removed_internal_index = removed_leaf_node.parent_index;
    const auto &removed_internal_node = m_nodes.at(removed_internal_index);

    auto sibling_index = removed_internal_node.child_index_2;
    if (leaf_index == removed_internal_node.child_index_2)
    {
        sibling_index = removed_internal_node.child_index_1;
    }
    auto &sibling_node = m_nodes.at(sibling_index);

    if (removed_internal_index != m_root_ind)
    {
        sibling_node.parent_index = removed_internal_node.parent_index;

        auto &internal_parent = m_nodes.at(removed_internal_node.parent_index);

        //! sibling of removed moves up closer to root
        internal_parent.child_index_1 == removed_internal_index ? internal_parent.child_index_1 = sibling_index : internal_parent.child_index_2 = sibling_index;

        //! parent of removed internal node needs to be refitted
        const auto &child1 = m_nodes.at(internal_parent.child_index_1);
        const auto &child2 = m_nodes.at(internal_parent.child_index_2);
        internal_parent.rect = makeUnion(child1.rect, child2.rect);
        internal_parent.height = 1 + std::max(child1.height, child2.height);
    }
    else
    { //! parent of leaf is root  -> the sibling is the new root
        sibling_node.parent_index = -1;
        m_root_ind = sibling_index;
    }

    //! deactivate removed node
    m_free_indices.insert(leaf_index);
    m_free_indices.insert(removed_internal_index);
    m_nodes.at(leaf_index).child_index_1 = -1;
    m_nodes.at(leaf_index).child_index_2 = -1;
    m_nodes.at(leaf_index).parent_index = -1;
    m_nodes.at(leaf_index).height = 0;
    if (removed_internal_index != -1)
    {
        m_nodes.at(removed_internal_index).child_index_1 = -1;
        m_nodes.at(removed_internal_index).child_index_2 = -1;
        m_nodes.at(removed_internal_index).parent_index = -1; //! refit volumes for ancenstors
        m_nodes.at(removed_internal_index).height = 0;
    }

    refitFrom(sibling_node.parent_index);
}

//! \brief refits bounding volumes of all parent nodes nodes
//! \brief  from \p starting_node_index towards root
void BoundingVolumeTree::refitFrom(int starting_node_index)
{
    int current_index = starting_node_index;
    while (current_index != -1)
    {
        current_index = balance(current_index);

        auto &current_node = m_nodes.at(current_index);
        const auto &child_1 = m_nodes.at(current_node.child_index_1);
        const auto &child_2 = m_nodes.at(current_node.child_index_2);
        current_node.rect = makeUnion(child_1.rect, child_2.rect);
        current_node.height = 1 + std::max(child_1.height, child_2.height);

        current_index = m_nodes.at(current_index).parent_index;
    }
}

//! \brief finds best sibling for the new_rect
//! \brief so as to minimize increase in tree volume upon addition
//! \brief should find the best option and thus makes higher quality trees but slower
int BoundingVolumeTree::findBestSibling(const AABB &new_rect)
{

    std::vector<std::pair<int, float>> to_visit;
    std::priority_queue pq(to_visit.begin(), to_visit.end(), [](const auto &p1, const auto &p2)
                           { return p1.second < p2.second; });

    pq.push({m_root_ind, 0});
    int best_index = m_root_ind;
    float best_cost = makeUnion(m_nodes.at(m_root_ind).rect, new_rect).volume();

    while (!pq.empty())
    {
        auto current_index = pq.top().first;
        auto cumulated_cost = pq.top().second;
        pq.pop();

        const auto &current_node = m_nodes.at(current_index);
        auto current_cost = makeUnion(current_node.rect, new_rect).volume();

        auto child1 = current_node.child_index_1;
        auto child2 = current_node.child_index_2;

        //! cost of changing current node
        auto cost_of_exchange = current_cost + cumulated_cost;

        if (cost_of_exchange < best_cost)
        {
            best_cost = cost_of_exchange;
            best_index = current_index;
        }

        cumulated_cost += current_cost - current_node.rect.volume();
        auto cost_lower_bound = new_rect.volume() + cumulated_cost;
        if (cost_lower_bound < best_cost && !current_node.isLeaf())
        {

            pq.push({child1, cumulated_cost});
            pq.push({child2, cumulated_cost});
        }
    }
    return best_index;
}

int BoundingVolumeTree::findBestSiblingExhaustive(const AABB &new_rect)
{

    std::stack<std::pair<int, float>> pq;
    pq.push({m_root_ind, 0});

    int best_index = m_root_ind;
    float best_cost = std::numeric_limits<float>::max();

    while (!pq.empty())
    {
        auto current_index = pq.top().first;
        auto cumulated_cost = pq.top().second;
        pq.pop();

        const auto &current_node = m_nodes.at(current_index);
        auto current_cost = makeUnion(current_node.rect, new_rect).volume();

        auto child1 = current_node.child_index_1;
        auto child2 = current_node.child_index_2;

        //! cost of changing current node
        auto cost_of_exchange = current_cost + cumulated_cost;

        if (cost_of_exchange < best_cost)
        {
            best_cost = cost_of_exchange;
            best_index = current_index;
        }

        cumulated_cost += current_cost - current_node.rect.volume();
        if (!current_node.isLeaf())
        {
            pq.push({child1, cumulated_cost});
            pq.push({child2, cumulated_cost});
        }
    }
    return best_index;
}

//! \brief tries to find best sibling for the new_rect
//! \brief so as to minimize increase in tree volume upon addition
//! \brief does not necessarily find the best option, but is faster and generally good enough
int BoundingVolumeTree::findBestSiblingGreedy(const AABB &new_rect)
{
    auto calcCostChange = [&new_rect, this](int node_index)
    {
        return makeUnion(m_nodes.at(node_index).rect, new_rect).volume() - m_nodes.at(node_index).rect.volume();
    };

    int current_index = m_root_ind;
    while (!m_nodes.at(current_index).isLeaf())
    {
        auto current_cost = makeUnion(m_nodes.at(current_index).rect, new_rect).volume();
        auto combined_vol = makeUnion(m_nodes.at(current_index).rect, new_rect).volume();
        float vol = m_nodes.at(current_index).rect.volume();
        float cumulated_cost = (combined_vol - vol);
        float cost = combined_vol;

        auto child1 = m_nodes.at(current_index).child_index_1;
        auto child2 = m_nodes.at(current_index).child_index_2;

        //! cost of changing current node
        auto cost_of_exchange = current_cost + cumulated_cost;

        float cost_1;
        auto new_vol = makeUnion(m_nodes.at(child1).rect, new_rect).volume();
        if (!m_nodes.at(child1).isLeaf())
        {
            cost_1 = new_vol + cumulated_cost;
        }
        else
        {
            auto old_vol = m_nodes.at(child1).rect.volume();
            cost_1 = new_vol - old_vol + cumulated_cost;
        }

        float cost_2;
        new_vol = makeUnion(m_nodes.at(child2).rect, new_rect).volume();
        if (!m_nodes.at(child2).isLeaf())
        {
            cost_2 = new_vol + cumulated_cost;
        }
        else
        {
            auto old_vol = m_nodes.at(child2).rect.volume();
            cost_2 = new_vol - old_vol + cumulated_cost;
        }
        if (cost < cost_1 && cost < cost_2)
        {
            break;
        }
        if (cost_1 < cost_2)
        {
            current_index = child1;
        }
        else
        {
            current_index = child2;
        }
    }
    return current_index;
}

bool BoundingVolumeTree::isLeaf(int node_index) const
{
    return m_nodes.at(node_index).child_index_1 == -1 && m_nodes.at(node_index).child_index_2 == -1;
}

//! \brief checks if tree contains cycles thus not being a tree (good for debugging :D)
bool BoundingVolumeTree::containsCycle() const
{
    std::vector<bool> visited(m_nodes.size(), false);
    std::stack<int> to_visit;
    to_visit.push(m_root_ind);
    while (!to_visit.empty())
    {
        auto current = to_visit.top();
        to_visit.pop();
        if (visited[current])
        {
            return true;
        }
        visited[current] = true;
        if (m_nodes.at(current).child_index_1 != -1)
        {
            to_visit.push(m_nodes.at(current).child_index_1);
            to_visit.push(m_nodes.at(current).child_index_2);
        }
    }
    return false;
}

//! \brief for every node checks if the child of the node has parent that is the node
bool BoundingVolumeTree::isConsistent() const
{
    std::vector<bool> visited(m_nodes.size(), false);
    std::stack<int> to_visit;
    to_visit.push(m_root_ind);

    for (auto &[entity_ind, node_ind] : m_object2node_indices)
    {
        if (m_nodes.at(node_ind).child_index_1 != -1)
        {
            return node_ind != m_nodes.at(m_nodes.at(node_ind).child_index_1).parent_index;
        }
        if (m_nodes.at(node_ind).child_index_2 != -1)
        {
            return node_ind != m_nodes.at(m_nodes.at(node_ind).child_index_2).parent_index;
        }

        if (m_nodes.at(node_ind).parent_index == -1 && m_root_ind != node_ind)
        {
            return false;
        }
    }

    return true;
}

//! \brief does tree rotation on subtree at node_index \p index_a
//! \brief in order to make it more balanced
//! \returns index of the node that moved at position of node at \p index_a
int BoundingVolumeTree::balance(int index_a)
{

    auto &node_a = m_nodes.at(index_a);
    if (node_a.isLeaf() || node_a.height <= 1)
    {
        return index_a;
    }

    auto index_b = m_nodes.at(index_a).child_index_1;
    auto index_c = m_nodes.at(index_a).child_index_2;
    auto &node_c = m_nodes.at(index_c);
    auto &node_b = m_nodes.at(index_b);

    auto root_node_height = m_nodes.at(m_root_ind).height;
    auto height_diff = node_c.height - node_b.height;
    if (height_diff > 1)
    { //! C is higher so it needs to move closer to root
        moveNodeUp(index_c);
        return index_c;
    }
    if (height_diff < -1)
    {
        moveNodeUp(index_b);
        return index_b;
    }
    return index_a;
}

//! \brief does a subtree rotation of subtree rooted at \p going_up_index  to reduce tree depth
//! \brief node C is put on As place, node A moves to Bs place and his new parent is C
//! \brief Cs children are thus A and higher node from C1 and C2 (has larger node.height)
//! \brief As children are thus B and lower node from C1 and C2 (has smaller node.height)
//! \brief Bs parent is A and his children stay the same
//! \brief     Example, old tree:
//! \brief
//! \brief           A
//! \brief       /       \
//! \brief      B         C
//! \brief    /   \     /   \  
//! \brief   B1   B2   C1   C2
//! \brief
//! \brief          assuming C2.height > C1.height, new tree is
//! \brief
//! \brief          C
//! \brief       /     \
//! \brief      A       C2
//! \brief     / \                                      
//! \brief    B   C1
//! \brief   / \      
//! \brief  B1  B2
void BoundingVolumeTree::moveNodeUp(int going_up_index)
{

    auto &node_c = m_nodes.at(going_up_index);
    auto index_a = node_c.parent_index;
    auto &node_a = m_nodes.at(index_a);

    bool switched = false;
    int index_b = node_a.child_index_1;
    if (node_a.child_index_1 == going_up_index) //! find which child of a is going_up
    {
        index_b = node_a.child_index_2;
        switched = true;
    }
    auto &node_b = m_nodes.at(index_b);

    auto index_c1 = node_c.child_index_1;
    auto index_c2 = node_c.child_index_2;

    auto &node_c1 = m_nodes.at(index_c1);
    auto &node_c2 = m_nodes.at(index_c2);

    node_c.parent_index = node_a.parent_index;
    node_a.parent_index = going_up_index;
    node_c.child_index_1 = index_a;

    if (node_c.parent_index != -1) //! tell As old parent that he has a new kid
    {
        if (m_nodes.at(node_c.parent_index).child_index_1 == index_a)
        {
            m_nodes.at(node_c.parent_index).child_index_1 = going_up_index;
        }
        else
        {
            m_nodes.at(node_c.parent_index).child_index_2 = going_up_index;
        }
    }
    else
    { //! c is now root;
        m_root_ind = going_up_index;
    }

    if (node_c1.height > node_c2.height) //! c2 becomes child of A
    {
        node_c.child_index_2 = index_c1;
        node_c2.parent_index = index_a;
        if (!switched)
        {
            node_a.child_index_2 = index_c2;
        }
        else
        {
            node_a.child_index_1 = index_c2;
        }
        node_a.height = 1 + std::max(node_c2.height, node_b.height);
        node_c.height = 1 + std::max(node_c1.height, node_a.height);

        node_a.rect = makeUnion(node_b.rect, node_c2.rect);
        node_c.rect = makeUnion(node_a.rect, node_c1.rect);
    }
    else //! c1 becomes child of A
    {
        node_c.child_index_2 = index_c2;
        node_c1.parent_index = index_a;
        if (!switched)
        {
            node_a.child_index_2 = index_c1;
        }
        else
        {
            node_a.child_index_1 = index_c1;
        }

        node_a.height = 1 + std::max(node_c1.height, node_b.height);
        node_c.height = 1 + std::max(node_c2.height, node_a.height);

        node_a.rect = makeUnion(node_b.rect, node_c1.rect);
        node_c.rect = makeUnion(node_a.rect, node_c2.rect);
    }
}

//! \brief finds object indices that intersect a given \p rect
std::vector<int> BoundingVolumeTree::findIntersectingLeaves(AABB rect)
{
    std::vector<int> intersecting_leaves;
    if (m_object2node_indices.empty()) //! if there are no objects there can be no intersections
    {
        return {};
    }

    std::stack<int> to_visit;
    auto &current_node = m_nodes.at(m_root_ind);
    to_visit.push(m_root_ind);
    while (!to_visit.empty())
    {
        auto current_index = to_visit.top();
        to_visit.pop();
        const auto &current_node = m_nodes.at(current_index);
        if (intersects(rect, current_node.rect))
        {

            if (current_node.child_index_1 != -1)
            {
                to_visit.push(current_node.child_index_1);
            }
            if (current_node.child_index_2 != -1)
            {
                to_visit.push(current_node.child_index_2);
            }

            if (current_node.isLeaf())
            {
                assert(current_node.object_index != -1);
                assert(m_object2node_indices.at(current_node.object_index) == current_index);
                intersecting_leaves.push_back(current_node.object_index);
            }
        }
    }

    return intersecting_leaves;
}

int BoundingVolumeTree::calcMaxDepth() const
{
    std::queue<std::pair<int, int>> to_visit;
    to_visit.push({m_root_ind, 0});
    int max_lvl = 0;
    while (!to_visit.empty())
    {

        auto current_index = to_visit.front().first;
        auto lvl = to_visit.front().second;
        to_visit.pop();
        if (lvl > max_lvl)
        {
            max_lvl = lvl;
        }
        auto &current = m_nodes.at(current_index);
        if (!current.isLeaf())
        {
            auto &child1 = m_nodes.at(current.child_index_1);
            auto &child2 = m_nodes.at(current.child_index_2);
            to_visit.push({current.child_index_1, lvl + 1});
            to_visit.push({current.child_index_2, lvl + 1});
        }
    }
    assert(max_lvl == m_nodes.at(m_root_ind).height);
    return max_lvl;
}

//! \brief finds if segment intersects given rectangle
//! \param from starting point of the segment
//! \param to end point of the segment
//! \param rect
//! \returns true if there is an intersection
bool BoundingVolumeTree::intersectsLine(sf::Vector2f from, sf::Vector2f to, AABB rect)
{

    auto dr = to - from;

    sf::Vector2f normal = {dr.y, -dr.x};
    normal /= norm(normal);

    sf::Vector2f r1 = rect.r_min;
    sf::Vector2f r2 = rect.r_min + sf::Vector2f{rect.r_max.x - rect.r_min.x, 0};
    sf::Vector2f r3 = rect.r_max;
    sf::Vector2f r4 = rect.r_min + sf::Vector2f{0, rect.r_max.y - rect.r_min.y};

    auto proj = projectOnAxis(normal, {r1, r2, r3, r4});
    auto proj2 = projectOnAxis(normal, {from});
    return overlap1D(proj, proj2);
}

//! \brief finds objects whose bounding rects intersect given a segment
//! \param from starting point of the segment
//! \param dir direction vector of the line
//! \param length of the segment
//! \returns list of object indices
std::vector<int> BoundingVolumeTree::rayCast(sf::Vector2f from, sf::Vector2f dir, float length)
{

    sf::Vector2f to = from + dir * length;
    std::vector<int> intersections;

    std::queue<int> to_visit;
    to_visit.push(m_root_ind);
    while (!to_visit.empty())
    {

        int current_ind = to_visit.front();
        to_visit.pop();
        auto &current = m_nodes.at(current_ind);

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

void BoundingVolumeTree::clear()
{
    m_object2node_indices.clear();
    for (int i = 0; i < m_nodes.size(); ++i)
    {
        m_free_indices.insert(i);
    }
    m_root_ind = -1;
}

int BoundingVolumeTree::maxBalanceFactor() const
{
    int max_balance = 0;
    std::queue<int> to_visit;

    to_visit.push(m_root_ind);
    while (!to_visit.empty())
    {
        auto &node = m_nodes.at(to_visit.front());
        to_visit.pop();
        if (!node.isLeaf())
        {
            auto h1 = m_nodes.at(node.child_index_1).height;
            auto h2 = m_nodes.at(node.child_index_2).height;
            max_balance = std::max(max_balance, std::abs(h1 - h2));
            to_visit.push(node.child_index_1);
            to_visit.push(node.child_index_2);
        }
    }
    return max_balance;
}

//! \brief finds intersectings bounding rectangles accross this and \p tree
//! \returns list of object indices whose bounding rects intersect
std::vector<std::pair<int, int>> BoundingVolumeTree::findClosePairsWith(BoundingVolumeTree &tree)
{
    std::vector<std::pair<int, int>> close_pairs;

    for (auto &[object_ind, node_ind] : m_object2node_indices)
    {
        auto nearest_objects_inds = tree.findIntersectingLeaves(m_nodes.at(node_ind).rect);
        for (auto i : nearest_objects_inds)
        {
            if (i == object_ind)
            {
                continue;
            }
            close_pairs.push_back({std::min(object_ind, i), std::max(object_ind, i)});
        }
    }
    return close_pairs;
}

const AABB &BoundingVolumeTree::getObjectRect(int object_ind) const
{
    return m_nodes.at(m_object2node_indices.at(object_ind)).rect;
}

const std::unordered_map<int, int> &BoundingVolumeTree::getObjects() const
{
    return m_object2node_indices;
}

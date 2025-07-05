#pragma once

#include "GameObject.h"
#include "Utils/Grid.h"

#include <vector>
#include <unordered_map>
#include <memory>

#include <Utils/Vector2.h>

class GridNeighbourSearcher
{

public:
    GridNeighbourSearcher(float max_dist = 30,
                          utils::Vector2f center_pos = {500, 500},
                          utils::Vector2f box_size = {1000, 1000});

    void moveEntity(GameObject &entity);
    void removeEntity(int entity_ind);
    void insertEntity(GameObject &entity);
    void insertEntityAt(GameObject &entity, int grid_ind);
    void setCenterPos(const utils::Vector2f &new_pos);
    int calcGridIndex(const utils::Vector2f &pos) const;

    std::vector<GameObject *> getNeighboursOfExcept(utils::Vector2f center, float radius, int exception) const;
    std::vector<GameObject *> getNeighboursOf(utils::Vector2f center, float radius) const;

private:
    std::unique_ptr<utils::SearchGrid> m_grid;

    utils::Vector2f m_center_pos;

    std::vector<std::unordered_map<int, GameObject *>> m_grid2entities;
    std::unordered_map<int, int> m_entity2grid_ind;
};

template <class DataType, class IdType>
struct ContiguousColony
{

    void reserve(std::size_t new_size)
    {
        data.reserve(new_size);
        data_ind2id.reserve(new_size);
    }

    void insert(IdType id, DataType datum)
    {
        data.push_back(datum);
        data_ind2id.push_back(id);

        assert(id2data_ind.count(id) == 0);
        id2data_ind[id] = data.size() - 1;
    }

    void erase(IdType id)
    {
        assert(id2data_ind.count(id) != 0);
        std::size_t data_ind = id2data_ind.at(id);

        IdType swapped_id = data_ind2id.at(data.size() - 1);
        id2data_ind.at(swapped_id) = data_ind; //! swapped points to erased

        data.at(data_ind) = data.back();               //! swap
        data.pop_back();                               //! and pop
        data_ind2id.at(data_ind) = data_ind2id.back(); //! swap
        data_ind2id.pop_back();                        //! and pop

        id2data_ind.erase(id);
    }

    bool isEmpty() const
    {
        return data.empty();
    }

    public:
    std::vector<DataType> data;
    std::vector<IdType> data_ind2id;

private:
    std::unordered_map<IdType, std::size_t> id2data_ind;
};

class SparseGridNeighbourSearcher
{

public:
    SparseGridNeighbourSearcher(float max_radius = 30.f)
    : grid_size(max_radius)
    {
    }

    std::vector<int> getNeighbourList(int ind, utils::Vector2f pos, float max_radius)
    {
        assert(max_radius < grid_size.x);
        
        GridIndT grid_ind = {std::floor(pos.x / grid_size.x), std::floor(pos.y / grid_size.y)};

        std::vector<int> neighbours;
        neighbours.reserve(100);

        const float max_radius_sq = max_radius * max_radius;
        
        for(int dx = -1; dx <= 1; ++dx)
        {
            for(int dy = -1; dy <= 1; ++dy)
            {
                GridIndT neighbour_grid_ind = {grid_ind.first + dx, grid_ind.second + dy};
                const auto& data = m_grid.at(neighbour_grid_ind)->data;
                const auto& data_inds = m_grid.at(neighbour_grid_ind)->data_ind2id;

                for(std::size_t i = 0; i < data.size(); ++i)
                {
                    if(utils::norm2(pos - data[i]) < max_radius_sq)
                    {
                        neighbours.push_back(data_inds[i]);
                    }
                }

            }
        }
        return neighbours;
    }

    void move(utils::Vector2f old_pos, utils::Vector2f new_pos, int ind)
    {
        GridIndT old_ind = {std::floor(old_pos.x / grid_size.x), std::floor(old_pos.y / grid_size.y)};
        GridIndT new_ind = {std::floor(new_pos.x / grid_size.x), std::floor(new_pos.y / grid_size.y)};
    
        if(old_ind != new_ind)
        {
            remove(old_pos, ind);
            insertAt(new_pos, ind);
        }
    }

    void insertAt(utils::Vector2f pos, int ind)
    {
        GridIndT grid_ind = {std::floor(pos.x / grid_size.x), std::floor(pos.y / grid_size.y)};
        
        if(m_grid.count(grid_ind) == 0)
        {
            addNode(grid_ind);
        }
        
        m_grid.at(grid_ind)->insert(ind, pos);
    }
    
    void remove(utils::Vector2f pos, int entity_ind)
    {
        GridIndT grid_ind = {std::floor(pos.x / grid_size.x), std::floor(pos.y / grid_size.y)};

        m_grid.at(grid_ind)->erase(entity_ind);
        if(m_grid.at(grid_ind)->isEmpty())
        {
            removeNode(grid_ind);
        }
    }

private:
    using GridIndT = std::pair<int, int>;
    using GridNodeType = ContiguousColony<utils::Vector2f, int>;

    void removeNode(GridIndT grid_ind)
    {
        assert(m_grid.count(grid_ind) != 0);
        m_grid.erase(grid_ind);
    }

    void addNode(GridIndT grid_ind)
    {
        assert(m_grid.count(grid_ind) == 0);
        m_grid[grid_ind] = std::make_shared<GridNodeType>();
    }

private:
    utils::Vector2f grid_size;

    struct pair_hash
    {
        std::size_t operator()(GridIndT grid_ind) const
        {
            return (grid_ind.first ^ grid_ind.second) << 16 + (grid_ind.first - grid_ind.second);
        }
    };

    std::unordered_map<GridIndT, std::shared_ptr<GridNodeType>, pair_hash> m_grid;
};

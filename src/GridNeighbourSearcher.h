#pragma once

#include "GameObject.h"
#include "Utils/Grid.h"
#include "Utils/ContiguousColony.h"

#include <vector>
#include <unordered_map>
#include <memory>

#include <Utils/Vector2.h>
#include <DrawLayer.h>

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



template <class DataType = utils::Vector2f>
class SparseGridNeighbourSearcher
{

    using GridIndT = std::pair<int, int>; //! x and y coordinate of a grid
    using GridNodeType = ContiguousColony<DataType, int>;

public:
    SparseGridNeighbourSearcher(float max_radius = 30.f)
        : grid_size(max_radius)
    {
    }

    void clear()
    {
        m_grid.clear();
        m_id2grid_ind.clear();
    }

    void drawGrid(DrawLayer &layer)
    {
        auto &canvas = layer.m_canvas;

        for (auto &[grid_ind, p_data] : m_grid)
        {
            auto [gx, gy] = grid_ind;

            utils::Vector2f lower_l = {gx * grid_size.x, gy * grid_size.y};
            utils::Vector2f lower_r = {(gx + 1) * grid_size.x, gy * grid_size.y};
            utils::Vector2f upper_l = {gx * grid_size.x, (gy + 1) * grid_size.y};
            utils::Vector2f upper_r = {(gx + 1) * grid_size.x, (gy + 1) * grid_size.y};
            canvas.drawLineBatched(lower_l, lower_r, 0.5, {1,0,0,1});
            canvas.drawLineBatched(lower_r, upper_r, 0.5, {1,0,0,1});
            canvas.drawLineBatched(lower_l, upper_l, 0.5, {1,0,0,1});
            canvas.drawLineBatched(upper_l, upper_r, 0.5, {1,0,0,1});
        }
    }

    std::vector<std::pair<utils::Vector2f, int>> getNeighbourList(int ind, utils::Vector2f pos, float max_radius)
    {
        assert(max_radius <= grid_size.x);

        GridIndT grid_ind = {std::floor(pos.x / grid_size.x), std::floor(pos.y / grid_size.y)};

        std::vector<std::pair<utils::Vector2f, int>> neighbours;
        neighbours.reserve(100);

        const float max_radius_sq = max_radius * max_radius;

        for (int dx = -1; dx <= 1; ++dx)
        {
            for (int dy = -1; dy <= 1; ++dy)
            {
                GridIndT neighbour_grid_ind = {grid_ind.first + dx, grid_ind.second + dy};
                if (m_grid.count(neighbour_grid_ind) == 0)
                {
                    continue;
                }
                const auto &data = m_grid.at(neighbour_grid_ind)->data;
                const auto &data_inds = m_grid.at(neighbour_grid_ind)->data_ind2id;

                for (std::size_t i = 0; i < data.size(); ++i)
                {
                    //! if within range and not neighbour with itself
                    if (data_inds[i] != ind && utils::norm2(pos - data[i]) < max_radius_sq)
                    {
                        neighbours.push_back({data[i], data_inds[i]});
                    }
                }
            }
        }
        return neighbours;
    }

    void move(utils::Vector2f new_pos, int ind)
    {

        GridIndT old_grid_ind = m_id2grid_ind.at(ind);
        GridIndT new_ind = {std::floor(new_pos.x / grid_size.x), std::floor(new_pos.y / grid_size.y)};

        if (old_grid_ind != new_ind)
        {
            auto datum = m_grid.at(old_grid_ind)->get(ind);
            remove(old_grid_ind, ind);
            insertAt(new_pos, datum, ind);
        }
    }

    
    void insertAt(utils::Vector2f pos, DataType datum, int ind)
    {
        GridIndT grid_ind = {std::floor(pos.x / grid_size.x), std::floor(pos.y / grid_size.y)};

        if (m_grid.count(grid_ind) == 0)
        {
            addNode(grid_ind);
        }

        assert(m_id2grid_ind.count(ind) == 0);
        m_id2grid_ind[ind] = grid_ind;
        m_grid.at(grid_ind)->insert(ind, datum);
    }

    void remove(int entity_ind)
    {
        remove(m_id2grid_ind.at(entity_ind), entity_ind);
    }

    void remove(GridIndT grid_ind, int entity_ind)
    {
        m_id2grid_ind.erase(entity_ind);

        m_grid.at(grid_ind)->erase(entity_ind);
        if (m_grid.at(grid_ind)->isEmpty())
        {
            removeNode(grid_ind);
        }
    }

private:
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
            return ((grid_ind.first ^ grid_ind.second) << 16) + (grid_ind.first - grid_ind.second);
        }
    };

    std::unordered_map<GridIndT, std::shared_ptr<GridNodeType>, pair_hash> m_grid;
    std::unordered_map<int, GridIndT> m_id2grid_ind;
};

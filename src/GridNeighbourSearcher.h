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
    GridNeighbourSearcher(  float max_dist = 30,
                             utils::Vector2f center_pos= {500, 500},
                             utils::Vector2f box_size = {1000, 1000});

    void moveEntity(GameObject &entity);
    void removeEntity(int entity_ind);
    void insertEntity(GameObject &entity);
    void insertEntityAt(GameObject &entity, int grid_ind);
    void setCenterPos(const utils::Vector2f& new_pos);
    int calcGridIndex(const utils::Vector2f& pos)const;

    std::vector<GameObject *> getNeighboursOfExcept(utils::Vector2f center, float radius, int exception) const;
    std::vector<GameObject *> getNeighboursOf(utils::Vector2f center, float radius) const;

private:
    std::unique_ptr<utils::SearchGrid> m_grid;
    
    utils::Vector2f m_center_pos;

    std::vector<std::unordered_map<int, GameObject *>> m_grid2entities;
    std::unordered_map<int, int> m_entity2grid_ind;
};

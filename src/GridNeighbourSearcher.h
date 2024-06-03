#pragma once

#include "Utils/Grid.h"

#include <vector>
#include <unordered_map>
#include <memory>

#include "GameObject.h"
#include "Geometry.h"

class GridNeighbourSearcher
{

public:
    GridNeighbourSearcher(float max_dist = 30);

    void moveEntity(GameObject &entity);
    void removeEntity(int entity_ind);
    void insertEntity(GameObject &entity);
    void insertEntityAt(GameObject &entity, int grid_ind);

    std::vector<GameObject *> getNeighboursOfExcept(sf::Vector2f center, float radius, int exception) const;
    std::vector<GameObject *> getNeighboursOf(sf::Vector2f center, float radius) const;

private:
    std::unique_ptr<SearchGrid> m_grid;
    
    std::vector<std::unordered_map<int, GameObject *>> m_grid2entities;
    std::unordered_map<int, int> m_entity2grid_ind;
};

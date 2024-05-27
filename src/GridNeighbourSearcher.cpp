#include "GridNeighbourSearcher.h"

GridNeighbourSearcher::GridNeighbourSearcher(float max_dist)
{
    sf::Vector2f box_size = {Geometry::BOX[0], Geometry::BOX[1]};
    const sf::Vector2i n_cells = {static_cast<int>(box_size.x / max_dist) + 1,
                                  static_cast<int>(box_size.y / max_dist) + 1};

    m_grid = std::make_unique<SearchGrid>(n_cells, sf::Vector2f{max_dist, max_dist});
    m_grid2entities.resize(n_cells.x * n_cells.y);
}

void GridNeighbourSearcher::moveEntity(GameObject &entity)
{
    auto old_grid_ind = m_entity2grid_ind.at(entity.getId());
    auto new_grid_ind = m_grid->coordToCell(entity.getPosition());
    if (new_grid_ind != old_grid_ind)
    {
        removeEntity(entity.getId());
        insertEntityAt(entity, new_grid_ind);
    }
}

void GridNeighbourSearcher::removeEntity(int entity_ind)
{
    auto grid_ind = m_entity2grid_ind.at(entity_ind);
    m_grid2entities.at(grid_ind).erase(entity_ind);
    m_entity2grid_ind.erase(entity_ind);
}

void GridNeighbourSearcher::insertEntity(GameObject &entity)
{
    insertEntityAt(entity, m_grid->coordToCell(entity.getPosition()));
}

void GridNeighbourSearcher::insertEntityAt(GameObject &entity, int grid_ind)
{
    //! there should be no entity of this id existing
    assert(m_entity2grid_ind.count(entity.getId()) == 0);
    assert(m_grid2entities.at(grid_ind).count(entity.getId()) == 0);

    m_grid2entities.at(grid_ind)[entity.getId()] = &entity;
    m_entity2grid_ind[entity.getId()] = grid_ind;
}

std::vector<GameObject *> GridNeighbourSearcher::getNeighboursOfExcept(sf::Vector2f center, float radius, int exception) const
{
    std::vector<GameObject *> nearest_neighbours;

    auto grid_ind = m_grid->coordToCell(center);
    auto radius_sq = radius * radius;

    std::array<int, 9> nearest_cells;
    int n_nearest_cells;
    m_grid->calcNearestCells(grid_ind, nearest_cells, n_nearest_cells);
    nearest_cells[n_nearest_cells] = grid_ind;
    n_nearest_cells++;

    for (int i = 0; i < n_nearest_cells; ++i)
    {
        const auto &neighbours = m_grid2entities.at(nearest_cells.at(i));
        for (auto [neighbour_id, neighbour] : neighbours)
        {
            if (dist2(neighbour->getPosition(), center) < radius_sq && neighbour_id != exception)
            {
                nearest_neighbours.push_back(neighbour);
            }
        }
    }
    return nearest_neighbours;
}

std::vector<GameObject *> GridNeighbourSearcher::getNeighboursOf(sf::Vector2f center, float radius) const
{
    return getNeighboursOfExcept(center, radius, -1);
}

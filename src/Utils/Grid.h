#pragma once

#include <SFML/System/Vector2.hpp>

#include <array>

class Grid {

  public:
    sf::Vector2i n_cells_;
    sf::Vector2f cell_size_ = {0, 0};

  public:
    Grid(sf::Vector2i n_cells, sf::Vector2f cell_size);

    [[nodiscard]] int coordToCell(float x, float y) const;
    [[nodiscard]] int coordToCell(sf::Vector2f r) const;
    [[nodiscard]] int cellIndex(int ix, int iy) const;
    [[nodiscard]] int cellIndex(sf::Vector2i) const;

    [[nodiscard]] int cellCoordX(int cell_index) const;
    [[nodiscard]] int cellCoordY(int cell_index) const;

    [[nodiscard]] int cellCoordX(sf::Vector2f r_coord) const;
    [[nodiscard]] int cellCoordY(sf::Vector2f r_coord) const;

    [[nodiscard]] sf::Vector2i cellCoords(sf::Vector2f r_coord) const;
    [[nodiscard]] sf::Vector2i cellCoords(sf::Vector2i r_coord) const;
    [[nodiscard]] sf::Vector2i cellCoords(int cell_index) const;

    int getNCells()const;
};

//! \class represents grids that are used for searching for nearest neighbours
struct SearchGrid : Grid {


    SearchGrid(sf::Vector2i n_cells, sf::Vector2f cell_size);
    bool isInGrid(sf::Vector2i cell_coords) const;

    void calcNearestCells(const int cell_ind, std::array<int, 9>& nearest_neighbours, int& n_nearest_cells) const;
    void calcNearestCells2(const int cell_ind, std::array<int, 9>& nearest_neighbours, int& n_nearest_cells) const;
};

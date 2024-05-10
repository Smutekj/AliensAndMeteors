#pragma once

namespace Geometry{
    constexpr int N_CELLS[2] = {256, 256};
    constexpr int CELL_SIZE = 5;
    constexpr int BOX[2] = {CELL_SIZE*N_CELLS[0], CELL_SIZE*N_CELLS[1]};
}; //! Namespace Geometry


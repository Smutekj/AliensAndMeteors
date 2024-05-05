#pragma once

namespace Geometry{
    constexpr u_int16_t N_CELLS[2] = {32, 32};
    constexpr int CELL_SIZE = 5;
    constexpr int BOX[2] = {CELL_SIZE*N_CELLS[0], CELL_SIZE*N_CELLS[1]};
}; //! Namespace Geometry


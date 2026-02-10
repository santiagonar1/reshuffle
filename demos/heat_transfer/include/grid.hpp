#ifndef RESHUFFLE_GRID_HPP
#define RESHUFFLE_GRID_HPP

#include "matrix.hpp"

#include <rank_id.hpp>

namespace heat {
    using Grid = Matrix2D<double>;
    using RankGrid = Matrix2D<reshuffle::RankId>;
}// namespace heat

#endif//RESHUFFLE_GRID_HPP

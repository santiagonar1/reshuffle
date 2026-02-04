#ifndef RESHUFFLE_MATRIX_HPP
#define RESHUFFLE_MATRIX_HPP
#include <vector>

namespace heat {
    using Matrix2D = std::vector<std::vector<double>>;

    auto operator<<(std::ostream &os, const Matrix2D &grid) -> std::ostream &;
}// namespace heat

#endif//RESHUFFLE_MATRIX_HPP

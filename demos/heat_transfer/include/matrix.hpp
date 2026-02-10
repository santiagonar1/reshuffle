#ifndef RESHUFFLE_MATRIX_HPP
#define RESHUFFLE_MATRIX_HPP

#include <ostream>
#include <vector>

namespace heat {
    template<typename T>
    using Matrix2D = std::vector<std::vector<T>>;

    template<typename T>
    auto operator<<(std::ostream &os, const Matrix2D<T> &matrix) -> std::ostream & {
        os << "[";
        for (const auto &row: matrix) {
            auto delimiter = std::string{};
            os << "(";
            for (const auto &ele: row) { os << std::exchange(delimiter, " ") << ele; }
            os << ")";
        }
        os << "]";

        return os;
    }
}// namespace heat

#endif//RESHUFFLE_MATRIX_HPP

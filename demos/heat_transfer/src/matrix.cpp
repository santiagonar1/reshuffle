#include "matrix.hpp"

#include <ostream>

namespace heat {
    auto operator<<(std::ostream &os, const Matrix2D &matrix) -> std::ostream & {
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
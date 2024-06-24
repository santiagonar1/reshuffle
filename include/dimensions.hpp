#ifndef RESHUFFLE_DIMENSIONS_HPP
#define RESHUFFLE_DIMENSIONS_HPP

namespace reshuffle {
    struct Dimensions2D {
        const int num_rows{};
        const int num_columns{};

        [[nodiscard]] auto get_num_values() const { return num_rows * num_columns; }
    };
}// namespace reshuffle

#endif//RESHUFFLE_DIMENSIONS_HPP

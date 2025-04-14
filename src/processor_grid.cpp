#include "processor_grid.hpp"

namespace reshuffle::dev {
    ProcessorGrid::ProcessorGrid(const int num_processors) : _num_processors(num_processors) {}

    auto ProcessorGrid::get_num_processors() const -> int { return _num_processors; }

    auto ProcessorGrid::operator==(const ProcessorGrid &other) const -> bool {
        return _num_processors == other._num_processors;
    }
}// namespace reshuffle::dev
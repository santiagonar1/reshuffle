#include "context.hpp"

namespace reshuffle::dev {
    auto Context::operator==(const Context &other) const -> bool {
        return distribution == other.distribution and comm == other.comm;
    }
}// namespace reshuffle::dev
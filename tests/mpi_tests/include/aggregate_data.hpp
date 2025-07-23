#ifndef AGGREGATE_DATA_HPP
#define AGGREGATE_DATA_HPP

#include <ostream>
#include <string>

struct AggregateData {
    std::string _dummy_string{};
    int _dummy_int{};

    bool operator==(const AggregateData &) const = default;
};

inline std::ostream &operator<<(std::ostream &os, const AggregateData &data) {
    return os << "AggregateData{_dummy_string: " << data._dummy_string
              << ", _dummy_int: " << data._dummy_int << "}";
}

#endif//AGGREGATE_DATA_HPP

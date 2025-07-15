#ifndef AGGREGATE_DATA_HPP
#define AGGREGATE_DATA_HPP

#include <string>

struct AggregateData {
    std::string _dummy_string{};
    int _dummy_int{};

    bool operator==(const AggregateData &) const = default;
};

#endif//AGGREGATE_DATA_HPP

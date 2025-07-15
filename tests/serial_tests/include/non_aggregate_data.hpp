#ifndef NON_AGGREGATE_DATA_HPP
#define NON_AGGREGATE_DATA_HPP

#include <string>
#include <zpp_bits.h>

struct NonAggregateData {
    NonAggregateData() = default;
    NonAggregateData(std::string str, const int val)
        : _dummy_int(val), _dummy_string(std::move(str)) { /*...*/ }// Make non-aggregate.

    int _dummy_int{};
    std::string _dummy_string{};
};

struct NonAggregateDataSerializable {
    using serialize = zpp::bits::members<2>;// Two members

    NonAggregateDataSerializable() = default;
    NonAggregateDataSerializable(std::string str, const int val)
        : _dummy_int(val), _dummy_string(std::move(str)) { /*...*/ }// Make non-aggregate.

    int _dummy_int{};
    std::string _dummy_string{};
};

struct NonAggregateDataSerializableWithArgumentLookup {
    NonAggregateDataSerializableWithArgumentLookup() = default;
    NonAggregateDataSerializableWithArgumentLookup(std::string str, const int val)
        : _dummy_int(val), _dummy_string(std::move(str)) { /*...*/ }// Make non-aggregate.

    int _dummy_int{};
    std::string _dummy_string{};
};

// Add this line somewhere before the actual serialization happens.
auto serialize(const NonAggregateDataSerializableWithArgumentLookup &person)
        -> zpp::bits::members<2>;

#endif//NON_AGGREGATE_DATA_HPP

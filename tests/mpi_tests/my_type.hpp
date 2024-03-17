#ifndef RESHUFFLE_MY_TYPE_HPP
#define RESHUFFLE_MY_TYPE_HPP

#include <zpp_bits.h>

struct MyPOD {
    int _my_value{42};

public:
    bool operator==(const MyPOD &other) const {
        return _my_value == other._my_value;
    }
};

struct NonAggregate {
    using serialize = zpp::bits::members<1>;

    int _my_value{42};

public:
    bool operator==(const NonAggregate &other) const {
        return _my_value == other._my_value;
    }

    explicit NonAggregate(int value) : _my_value(value) {}

    [[nodiscard]] static NonAggregate create() {
        return NonAggregate(42);
    }
};

struct NonAggregateDefaultConstructible {
    using serialize = zpp::bits::members<1>;

    int _my_value{42};

public:
    bool operator==(const NonAggregateDefaultConstructible &other) const {
        return _my_value == other._my_value;
    }

    NonAggregateDefaultConstructible() = default;
    explicit NonAggregateDefaultConstructible(int value) : _my_value(value) {}
};

#endif //RESHUFFLE_MY_TYPE_HPP

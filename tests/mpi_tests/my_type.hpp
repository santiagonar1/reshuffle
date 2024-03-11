#ifndef RESHUFFLE_MY_TYPE_HPP
#define RESHUFFLE_MY_TYPE_HPP

struct MyPOD {
    int _my_value{42};

public:
    bool operator==(const MyPOD &other) const {
        return _my_value == other._my_value;
    }
};

#endif //RESHUFFLE_MY_TYPE_HPP

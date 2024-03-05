#ifndef RESHUFFLE_CONCEPTS_HPP
#define RESHUFFLE_CONCEPTS_HPP

namespace reshuffle {
    template<typename T>
    concept Container = requires(T &t) {
        typename T::value_type;
        t.data();
        t.size();
    };
}

#endif //RESHUFFLE_CONCEPTS_HPP

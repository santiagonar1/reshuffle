#ifndef RESHUFFLE_CONCEPTS_HPP
#define RESHUFFLE_CONCEPTS_HPP

namespace reshuffle {
    template<typename T>
    concept Iterable = requires(T t) {
        { std::begin(t) } -> std::forward_iterator;
        { std::end(t) } -> std::forward_iterator;
    };

    template<typename T>
    concept ContiguousContainer = Iterable<T> && requires(T &t) {
        typename T::value_type;
        t.data();
        t.size();
    };
}

#endif //RESHUFFLE_CONCEPTS_HPP

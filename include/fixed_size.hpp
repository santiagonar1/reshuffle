#ifndef FIXED_SIZE_HPP
#define FIXED_SIZE_HPP

namespace reshuffle::internal {
    template<typename T>
    struct serialization_traits_tag {
        static constexpr bool has_fixed_size = false;
    };

    template<typename T>
    constexpr bool is_fixed_size_serializable_v = serialization_traits_tag<T>::has_fixed_size;

}// namespace reshuffle::internal

#define RESHUFFLE_HAS_FIXED_SIZE_SERIALIZABLE(Type)                                                \
    template<>                                                                                     \
    struct reshuffle::internal::serialization_traits_tag<Type> {                                   \
        static constexpr bool has_fixed_size = true;                                               \
    };


#endif//FIXED_SIZE_HPP

#ifndef COMPLEX_DATA_HPP
#define COMPLEX_DATA_HPP

struct Base {
    Base() = default;
    explicit Base(const int id) : id_base(id) { /*...*/ }// Make non-aggregate.
    int id_base{};
};

struct Derived : Base {
    Derived() = default;
    explicit Derived(const int id_base, const int id)
        : Base(id_base), id_derived(id) { /*...*/ }// Make non-aggregate.

    static constexpr auto serialize(auto &archive, Derived &d) {
        return archive(d.id_base, d.id_derived);
    }

    static constexpr auto serialize(auto &archive, const Derived &d) {
        return archive(d.id_base, d.id_derived);
    }

    bool operator==(const Derived &other) const {
        return id_base == other.id_base and id_derived == other.id_derived;
    }

    int id_derived{};
};

#endif//COMPLEX_DATA_HPP

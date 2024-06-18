#ifndef RESHUFFLE_DATA_DISTRIBUTION_HPP
#define RESHUFFLE_DATA_DISTRIBUTION_HPP

#include "subdomain.hpp"
#include <vector>

namespace reshuffle {
    class BlockWise {
    private:
        const int _num_blocks;

        [[nodiscard]] int get_min_values_per_block(int num_values) const {
            return num_values / _num_blocks;
        }

    public:
        explicit BlockWise(int num_blocks) : _num_blocks(num_blocks) {}

        [[nodiscard]] auto get_subdomains(int num_values) const {
            const auto min_values_per_block = get_min_values_per_block(num_values);
            std::vector<Subdomain> subdomains{};

            for (int i = 0; i < min_values_per_block * _num_blocks; i += min_values_per_block) {
                const auto starting_index = i;
                const auto last_index = starting_index + min_values_per_block;
                subdomains.emplace_back(starting_index, last_index);
            }

            Subdomain last_block{subdomains.back().get_left_bound(), num_values};
            subdomains.pop_back();
            subdomains.push_back(last_block);
            return subdomains;
        }
    };
}// namespace reshuffle


#endif//RESHUFFLE_DATA_DISTRIBUTION_HPP

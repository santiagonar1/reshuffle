#ifndef RESHUFFLE_GENERAL_DATA_DISTRIBUTION_HPP
#define RESHUFFLE_GENERAL_DATA_DISTRIBUTION_HPP

#include "block.hpp"
#include "coordinates.hpp"
#include "data_distribution.hpp"
#include "grid_layout.hpp"
#include "multidimensional_interval.hpp"
#include "processor_grid.hpp"
#include "rank_id.hpp"

#include <expected>
#include <format>
#include <map>

namespace reshuffle {
    using IntervalId = std::size_t;
    using ErrorMessage = std::string;

    template<std::size_t N>
    using GlobalMapping = std::map<RankId, std::vector<MultidimensionalInterval<N>>>;

    namespace internal {
        [[nodiscard]] auto find_problematic_intervals(const std::vector<Interval> &intervals)
                -> std::optional<std::pair<Interval, Interval>>;

        template<std::size_t N>
        [[nodiscard]] auto get_intervals(const GlobalMapping<N> &global_mapping)
                -> std::vector<MultidimensionalInterval<N>>;

        template<std::size_t N>
        [[nodiscard]] auto get_max_rank(const GlobalMapping<N> &global_mapping) -> RankId;
    }// namespace internal


    template<std::size_t N>
    class GeneralDataDistribution final : public DataDistribution<N> {
    public:
        [[nodiscard]] static auto make(const GlobalMapping<N> &global_mapping,
                                       const std::map<IntervalId, Coordinates<N>> &local_mapping,
                                       RankId rank)
                -> std::expected<GeneralDataDistribution, ErrorMessage>;

        [[nodiscard]] auto get_grid_layout() const -> const GridLayout<N> & override;
        [[nodiscard]] auto get_processor_grid() const -> const ProcessorGrid<N> & override;
        [[nodiscard]] auto is_block_wise() const -> bool override;
        [[nodiscard]] auto clone() const -> std::unique_ptr<DataDistribution<N>> override;


    private:
        //
        const ProcessorGrid<N> _processor_grid;
        const GridLayout<N> _grid_layout;

        //
        struct Ok {};

        GeneralDataDistribution(const ProcessorGrid<N> &processor_grid,
                                const GridLayout<N> &grid_layout)
            : _processor_grid(processor_grid), _grid_layout(grid_layout) {};

        [[nodiscard]] static auto check_contiguity_intervals(const std::vector<Interval> &intervals)
                -> std::variant<Ok, ErrorMessage>;

        [[nodiscard]] static auto
        check_contiguity_intervals(const std::array<std::vector<Interval>, N> &intervals)
                -> std::variant<Ok, ErrorMessage>;

        [[nodiscard]] static auto check_consistency_local_and_global_mapping(
                const GlobalMapping<N> &global_mapping,
                const std::map<IntervalId, Coordinates<N>> &local_mapping, RankId rank)
                -> std::variant<Ok, ErrorMessage>;

        [[nodiscard]] static auto
        run_checks(const GlobalMapping<N> &global_mapping,
                   const std::map<IntervalId, Coordinates<N>> &local_mapping, RankId rank)
                -> std::variant<Ok, ErrorMessage>;

        [[nodiscard]] static auto create_processor_grid(RankId max_rank) -> ProcessorGrid<N>;

        [[nodiscard]] static auto create_processor_grid(const GlobalMapping<N> &global_mapping)
                -> ProcessorGrid<N>;

        [[nodiscard]] static auto create_blocks(const GlobalMapping<N> &global_mapping,
                                                const ProcessorGrid<N> &processor_grid)
                -> std::array<std::vector<Block>, N>;

        [[nodiscard]] static auto create_grid_layout(const GlobalMapping<N> &global_mapping,
                                                     const ProcessorGrid<N> &processor_grid)
                -> GridLayout<N>;
    };

    template<std::size_t N>
    auto GeneralDataDistribution<N>::make(const GlobalMapping<N> &global_mapping,
                                          const std::map<IntervalId, Coordinates<N>> &local_mapping,
                                          RankId rank)
            -> std::expected<GeneralDataDistribution, ErrorMessage> {
        if (const auto result = run_checks(global_mapping, local_mapping, rank);
            std::holds_alternative<ErrorMessage>(result)) {
            const auto error_msg =
                    std::format("[RANK {}] {}", rank, std::get<ErrorMessage>(result));
            return std::unexpected(error_msg);
        }

        const auto processor_grid = create_processor_grid(global_mapping);
        const auto grid_layout = create_grid_layout(global_mapping, processor_grid);

        return GeneralDataDistribution{processor_grid, grid_layout};
    }

    template<std::size_t N>
    auto GeneralDataDistribution<N>::get_grid_layout() const -> const GridLayout<N> & {
        return _grid_layout;
    }

    template<std::size_t N>
    auto GeneralDataDistribution<N>::get_processor_grid() const -> const ProcessorGrid<N> & {
        return _processor_grid;
    }

    template<std::size_t N>
    auto GeneralDataDistribution<N>::is_block_wise() const -> bool {
        //TODO: actually check if this is the case
        return false;
    }

    template<std::size_t N>
    auto GeneralDataDistribution<N>::clone() const -> std::unique_ptr<DataDistribution<N>> {
        return std::make_unique<GeneralDataDistribution>(*this);
    }

    //TODO: Should I check also for repeated intervals?
    template<std::size_t N>
    auto GeneralDataDistribution<N>::run_checks(
            const GlobalMapping<N> &global_mapping,
            const std::map<IntervalId, Coordinates<N>> &local_mapping, const RankId rank)
            -> std::variant<Ok, ErrorMessage> {
        const auto multidimensional_intervals = internal::get_intervals(global_mapping);
        const auto unidimensional_intervals =
                internal::to_unidimensional_intervals(multidimensional_intervals);

        if (const auto result = check_contiguity_intervals(unidimensional_intervals);
            std::holds_alternative<ErrorMessage>(result)) {
            return ErrorMessage(std::get<ErrorMessage>(result));
        }

        if (const auto result =
                    check_consistency_local_and_global_mapping(global_mapping, local_mapping, rank);
            std::holds_alternative<ErrorMessage>(result)) {
            return ErrorMessage(std::get<ErrorMessage>(result));
        }

        return Ok{};
    }

    template<std::size_t N>
    auto GeneralDataDistribution<N>::create_processor_grid(const RankId max_rank)
            -> ProcessorGrid<N> {
        auto processor_grid_dimensions = Dimensions<N>{};
        processor_grid_dimensions.fill(1);
        processor_grid_dimensions[0] = max_rank + 1;
        const auto processor_grid = ProcessorGrid{processor_grid_dimensions};

        return processor_grid;
    }

    template<std::size_t N>
    auto GeneralDataDistribution<N>::create_processor_grid(const GlobalMapping<N> &global_mapping)
            -> ProcessorGrid<N> {
        const auto max_rank = internal::get_max_rank(global_mapping);

        return create_processor_grid(max_rank);
    }

    template<std::size_t N>
    auto GeneralDataDistribution<N>::create_blocks(const GlobalMapping<N> &global_mapping,
                                                   const ProcessorGrid<N> &processor_grid)
            -> std::array<std::vector<Block>, N> {
        auto blocks = std::array<std::vector<Block>, N>{};
        for (const auto &[rank_id, intervals]: global_mapping) {
            for (const auto &interval: intervals) {
                const auto rank_coordinates = processor_grid.get_processor_coordinates(rank_id);
                const auto decomposed_interval = internal::to_unidimensional_intervals(interval);
                for (int dim = 0; dim < N; ++dim) {
                    blocks[dim].emplace_back(
                            Block{decomposed_interval[dim], rank_coordinates[dim]});
                }
            }
        }

        // I am not sure if this is necessary, but I do it to keep consistency with the other
        // distributions
        for (auto &vector_blocks: blocks) { std::ranges::sort(vector_blocks); }

        return blocks;
    }

    template<std::size_t N>
    auto GeneralDataDistribution<N>::create_grid_layout(const GlobalMapping<N> &global_mapping,
                                                        const ProcessorGrid<N> &processor_grid)
            -> GridLayout<N> {
        const auto blocks = create_blocks(global_mapping, processor_grid);
        return GridLayout{blocks};
    }

    template<std::size_t N>
    auto
    GeneralDataDistribution<N>::check_contiguity_intervals(const std::vector<Interval> &intervals)
            -> std::variant<Ok, ErrorMessage> {
        const auto problematic_intervals_opt = internal::find_problematic_intervals(intervals);
        if (not problematic_intervals_opt.has_value()) { return Ok{}; }

        const auto [first, second] = problematic_intervals_opt.value();
        return std::format("Intervals {} and {} are not contiguous ", first.to_string(),
                           second.to_string());
    }

    template<std::size_t N>
    auto GeneralDataDistribution<N>::check_contiguity_intervals(
            const std::array<std::vector<Interval>, N> &intervals)
            -> std::variant<Ok, ErrorMessage> {
        for (const auto &vector_intervals: intervals) {
            if (const auto result = check_contiguity_intervals(vector_intervals);
                std::holds_alternative<ErrorMessage>(result)) {
                return result;
            }
        }

        return Ok{};
    }

    template<std::size_t N>
    auto GeneralDataDistribution<N>::check_consistency_local_and_global_mapping(
            const GlobalMapping<N> &global_mapping,
            const std::map<IntervalId, Coordinates<N>> &local_mapping, RankId rank)
            -> std::variant<Ok, ErrorMessage> {
        const auto global_assigned_intervals = global_mapping.contains(rank)
                                                       ? global_mapping.at(rank)
                                                       : std::vector<MultidimensionalInterval<N>>{};
        if (global_assigned_intervals.size() != local_mapping.size()) {
            const auto error_msg =
                    std::format("global_mapping assigned {} blocks to rank, but {} "
                                "are listed in local_mapping",
                                rank, global_assigned_intervals.size(), local_mapping.size());
            return ErrorMessage{error_msg};
        }

        return Ok{};
    }

    namespace internal {
        template<std::size_t N>
        auto get_intervals(const GlobalMapping<N> &global_mapping)
                -> std::vector<MultidimensionalInterval<N>> {
            auto intervals = std::vector<MultidimensionalInterval<N>>{};
            for (const auto &[rank, multidimensional_intervals]: global_mapping) {
                intervals.insert_range(intervals.end(), multidimensional_intervals);
            }

            return intervals;
        }

        template<std::size_t N>
        auto get_max_rank(const GlobalMapping<N> &global_mapping) -> RankId {
            if (global_mapping.empty()) { return INVALID_RANK_ID; }

            auto keys = global_mapping | std::views::keys;
            const auto max_rank = std::ranges::max_element(keys.begin(), keys.end());
            return *max_rank;
        }
    }// namespace internal

}// namespace reshuffle

#endif//RESHUFFLE_GENERAL_DATA_DISTRIBUTION_HPP

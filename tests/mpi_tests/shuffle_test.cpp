#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <shuffle.hpp>

using namespace reshuffle;
using namespace reshuffle::mpi;

using testing::Eq;

[[nodiscard]] auto generate_values(int from, int to) -> std::vector<int>;

class ValuesGenerator {
public:
    ValuesGenerator(const int num_values_per_rank, const int num_ranks)
        : _num_values_per_rank(num_values_per_rank), _num_ranks(num_ranks),
          _values(generate_values(1, num_values_per_rank * num_ranks)) {}

    [[nodiscard]] auto get_values_for_rank(const reshuffle::rank_id rank_id) const
            -> std::vector<int> {
        const auto start = rank_id * _num_values_per_rank;
        const auto end = start + _num_values_per_rank;

        return {_values.begin() + start, _values.begin() + end};
    };

    [[nodiscard]] auto get_all_values() const -> std::vector<int> { return _values; }

    [[nodiscard]] auto get_total_num_values() const -> int {
        return _num_ranks * _num_values_per_rank;
    }

private:
    const int _num_values_per_rank;
    const int _num_ranks;
    const std::vector<int> _values;
};

enum class CommSelector {
    ONLY_RANK_0,
    ONLY_RANK_1,
    ALL_RANKS,
};

enum class DataLocationSelector {
    ONLY_RANK_0,
    ONLY_RANK_1,
    ALL_RANKS,
};

[[nodiscard]] auto create_communicator(const CommSelector &comm_selector) -> MPI_Comm;
[[nodiscard]] auto create_context(const DataLocationSelector &data_location,
                                  const CommSelector &comm_selector, int num_global_values)
        -> Context<1>;
[[nodiscard]] auto create_context(const DataLocationSelector &data_location, MPI_Comm comm,
                                  int num_global_values) -> Context<1>;
[[nodiscard]] auto is_disjoint(const DataLocationSelector &data_location,
                               const CommSelector &comm_selector) -> bool;
[[nodiscard]] auto is_rank_with_data_outside_comm(const DataLocationSelector &data_location,
                                                  const CommSelector &comm_selector) -> bool;

TEST(Shuffle, CanShuffleFromOneToMany) {
    constexpr auto num_values_per_rank = 6;
    const auto num_ranks = get_num_ranks(MPI_COMM_WORLD);
    const auto rank = get_rank_id(MPI_COMM_WORLD);

    const auto generator = ValuesGenerator(num_values_per_rank, num_ranks);
    const auto num_global_values = generator.get_total_num_values();

    const auto values = is_root(MPI_COMM_WORLD) ? generator.get_all_values() : std::vector<int>();

    const auto initial_context = create_context(DataLocationSelector::ONLY_RANK_0,
                                                CommSelector::ALL_RANKS, num_global_values);
    const auto final_context = create_context(DataLocationSelector::ALL_RANKS,
                                              CommSelector::ALL_RANKS, num_global_values);

    const auto new_values =
            shuffle(std::mdspan{values.data(), values.size()}, initial_context, final_context)
                    .first;

    EXPECT_THAT(new_values, Eq(generator.get_values_for_rank(rank)));
}

TEST(Shuffle, CanShuffleFromOneToManyIn2DVerticalSplit) {
    const auto values =
            is_root(MPI_COMM_WORLD) ? std::vector{0, 1, 2, 3, 4, 5} : std::vector<int>();
    constexpr auto num_global_rows = 2;
    constexpr auto num_global_columns = 3;
    const auto num_rows = is_root(MPI_COMM_WORLD) ? num_global_rows : 0;
    const auto num_columns = is_root(MPI_COMM_WORLD) ? num_global_columns : 0;


    const auto comm = create_communicator(CommSelector::ALL_RANKS);
    const auto initial_context =
            Context{make_block_wise_distribution({num_global_rows, num_global_columns},
                                                 ProcessorGrid<2>{{1, 1}}),
                    comm};
    const auto final_context =
            Context{make_block_wise_distribution({num_global_rows, num_global_columns},
                                                 ProcessorGrid<2>{{1, 2}}),
                    comm};

    const auto [new_values, local_dimensions] = shuffle(
            std::mdspan{values.data(), num_rows, num_columns}, initial_context, final_context);

    if (is_root(MPI_COMM_WORLD)) {
        EXPECT_THAT(new_values, Eq(std::vector{0, 1, 3, 4}));
        EXPECT_THAT(local_dimensions, Eq(reshuffle::Dimensions<2>{2, 2}));
    } else {
        EXPECT_THAT(new_values, Eq(std::vector{2, 5}));
        EXPECT_THAT(local_dimensions, Eq(reshuffle::Dimensions<2>{2, 1}));
    }
}

TEST(Shuffle, CanShuffleFromOneToManyIn2DVerticalSplitVectorOfVectors) {
    const auto values = is_root(MPI_COMM_WORLD)
                                ? std::vector{std::vector{0, 1, 2}, std::vector{3, 4, 5}}
                                : std::vector<std::vector<int>>();
    constexpr auto num_global_rows = 2;
    constexpr auto num_global_columns = 3;


    const auto comm = create_communicator(CommSelector::ALL_RANKS);
    const auto initial_context =
            Context{make_block_wise_distribution({num_global_rows, num_global_columns},
                                                 ProcessorGrid<2>{{1, 1}}),
                    comm};
    const auto final_context =
            Context{make_block_wise_distribution({num_global_rows, num_global_columns},
                                                 ProcessorGrid<2>{{1, 2}}),
                    comm};

    const auto [new_values, local_dimensions] = shuffle(values, initial_context, final_context);

    if (is_root(MPI_COMM_WORLD)) {
        EXPECT_THAT(new_values, Eq(std::vector{0, 1, 3, 4}));
        EXPECT_THAT(local_dimensions, Eq(reshuffle::Dimensions<2>{2, 2}));
    } else {
        EXPECT_THAT(new_values, Eq(std::vector{2, 5}));
        EXPECT_THAT(local_dimensions, Eq(reshuffle::Dimensions<2>{2, 1}));
    }
}

TEST(Shuffle, CanShuffleFromOneToManyIn2DHorizontalSplit) {
    const auto values =
            is_root(MPI_COMM_WORLD) ? std::vector{0, 1, 2, 3, 4, 5} : std::vector<int>();
    constexpr auto num_global_rows = 2;
    constexpr auto num_global_columns = 3;
    const auto num_rows = is_root(MPI_COMM_WORLD) ? num_global_rows : 0;
    const auto num_columns = is_root(MPI_COMM_WORLD) ? num_global_columns : 0;


    const auto comm = create_communicator(CommSelector::ALL_RANKS);
    const auto initial_context =
            Context{make_block_wise_distribution({num_global_rows, num_global_columns},
                                                 ProcessorGrid<2>{{1, 1}}),
                    comm};
    const auto final_context =
            Context{make_block_wise_distribution({num_global_rows, num_global_columns},
                                                 ProcessorGrid<2>{{2, 1}}),
                    comm};

    const auto [new_values, local_dimensions] = shuffle(
            std::mdspan{values.data(), num_rows, num_columns}, initial_context, final_context);

    if (is_root(MPI_COMM_WORLD)) {
        EXPECT_THAT(new_values, Eq(std::vector{0, 1, 2}));
        EXPECT_THAT(local_dimensions, Eq(reshuffle::Dimensions<2>{1, 3}));
    } else {
        EXPECT_THAT(new_values, Eq(std::vector{3, 4, 5}));
        EXPECT_THAT(local_dimensions, Eq(reshuffle::Dimensions<2>{1, 3}));
    }
}

TEST(Shuffle, CanShuffleFromManyToOne) {
    constexpr auto num_values_per_rank = 6;
    const auto num_ranks = get_num_ranks(MPI_COMM_WORLD);
    const auto rank = get_rank_id(MPI_COMM_WORLD);

    const auto generator = ValuesGenerator(num_values_per_rank, num_ranks);
    const auto num_global_values = generator.get_total_num_values();

    const auto values = generator.get_values_for_rank(rank);

    const auto initial_context = create_context(DataLocationSelector::ALL_RANKS,
                                                CommSelector::ALL_RANKS, num_global_values);
    const auto final_context = create_context(DataLocationSelector::ONLY_RANK_0,
                                              CommSelector::ALL_RANKS, num_global_values);

    const auto new_values =
            shuffle(std::mdspan{values.data(), values.size()}, initial_context, final_context)
                    .first;

    if (is_root(MPI_COMM_WORLD)) {
        EXPECT_THAT(new_values, Eq(generator.get_all_values()));
    } else {
        EXPECT_TRUE(new_values.empty());
    }
}

TEST(Shuffle, CanShuffleFromManyToOneIn2DVerticalSplit) {
    const auto values = is_root(MPI_COMM_WORLD) ? std::vector{0, 1, 3, 4} : std::vector{2, 5};
    constexpr auto num_global_rows = 2;
    constexpr auto num_global_columns = 3;
    constexpr auto num_rows = 2;
    const auto num_columns = is_root(MPI_COMM_WORLD) ? 2 : 1;

    const auto comm = create_communicator(CommSelector::ALL_RANKS);
    const auto initial_context =
            Context{make_block_wise_distribution({num_global_rows, num_global_columns},
                                                 ProcessorGrid<2>{{1, 2}}),
                    comm};
    const auto final_context =
            Context{make_block_wise_distribution({num_global_rows, num_global_columns},
                                                 ProcessorGrid<2>{{1, 1}}),
                    comm};

    const auto [new_values, local_dimensions] = shuffle(
            std::mdspan{values.data(), num_rows, num_columns}, initial_context, final_context);

    if (is_root(MPI_COMM_WORLD)) {
        EXPECT_THAT(new_values, Eq(std::vector{0, 1, 2, 3, 4, 5}));
        EXPECT_THAT(local_dimensions,
                    Eq(reshuffle::Dimensions<2>{num_global_rows, num_global_columns}));
    } else {
        EXPECT_TRUE(new_values.empty());
        EXPECT_THAT(local_dimensions, Eq(reshuffle::Dimensions<2>{0, 0}));
    }
}

TEST(Shuffle, CanShuffleFromManyToOneIn2DHorizontalSplit) {
    const auto values = is_root(MPI_COMM_WORLD) ? std::vector{0, 1, 2} : std::vector{3, 4, 5};
    constexpr auto num_global_rows = 2;
    constexpr auto num_global_columns = 3;
    constexpr auto num_rows = 1;
    constexpr auto num_columns = 3;

    const auto comm = create_communicator(CommSelector::ALL_RANKS);
    const auto initial_context =
            Context{make_block_wise_distribution({num_global_rows, num_global_columns},
                                                 ProcessorGrid<2>{{2, 1}}),
                    comm};
    const auto final_context =
            Context{make_block_wise_distribution({num_global_rows, num_global_columns},
                                                 ProcessorGrid<2>{{1, 1}}),
                    comm};

    const auto [new_values, local_dimensions] = shuffle(
            std::mdspan{values.data(), num_rows, num_columns}, initial_context, final_context);

    if (is_root(MPI_COMM_WORLD)) {
        EXPECT_THAT(new_values, Eq(std::vector{0, 1, 2, 3, 4, 5}));
        EXPECT_THAT(local_dimensions,
                    Eq(reshuffle::Dimensions<2>{num_global_rows, num_global_columns}));
    } else {
        EXPECT_TRUE(new_values.empty());
        EXPECT_THAT(local_dimensions, Eq(reshuffle::Dimensions<2>{0, 0}));
    }
}

TEST(Shuffle, CanShuffleFromBlockWiseToBlockCyclic) {
    constexpr auto num_values_per_rank = 6;
    const auto num_ranks = get_num_ranks(MPI_COMM_WORLD);
    const auto rank = get_rank_id(MPI_COMM_WORLD);

    const auto generator = ValuesGenerator(num_values_per_rank, num_ranks);
    const auto num_global_values = generator.get_total_num_values();

    const auto values = generator.get_values_for_rank(rank);

    const auto initial_context = create_context(DataLocationSelector::ALL_RANKS,
                                                CommSelector::ALL_RANKS, num_global_values);

    const auto final_processor_grid = ProcessorGrid<1>{{num_ranks}};
    const auto final_distribution = BlockCyclic{{num_global_values}, {4}, final_processor_grid};
    const auto final_context = Context{final_distribution, MPI_COMM_WORLD};

    const auto new_values =
            shuffle(std::mdspan{values.data(), values.size()}, initial_context, final_context)
                    .first;

    if (is_root(MPI_COMM_WORLD)) {
        const auto all_values = generator.get_all_values();
        const auto expected_new_values =
                std::vector{all_values[0], all_values[1], all_values[2],  all_values[3],
                            all_values[8], all_values[9], all_values[10], all_values[11]};
        EXPECT_THAT(new_values, Eq(expected_new_values));
    } else {
        const auto all_values = generator.get_all_values();
        const auto expected_new_values =
                std::vector{all_values[4], all_values[5], all_values[6], all_values[7]};
        EXPECT_THAT(new_values, Eq(expected_new_values));
    }
}

TEST(Shuffle, CanShuffleFromBlockWiseToBlockCyclicIn2D) {
    const auto values = is_root(MPI_COMM_WORLD) ? std::vector{0, 1, 2} : std::vector{3, 4, 5};
    constexpr auto num_global_rows = 2;
    constexpr auto num_global_columns = 3;
    constexpr auto num_rows = 1;
    constexpr auto num_columns = 3;

    const auto comm = create_communicator(CommSelector::ALL_RANKS);
    const auto initial_context =
            Context{make_block_wise_distribution({num_global_rows, num_global_columns},
                                                 ProcessorGrid<2>{{2, 1}}),
                    comm};
    const auto final_distribution =
            BlockCyclic{{num_global_rows, num_global_columns}, {2, 1}, ProcessorGrid<2>{{1, 2}}};
    const auto final_context = Context{final_distribution, comm};

    const auto [new_values, local_dimensions] = shuffle(
            std::mdspan{values.data(), num_rows, num_columns}, initial_context, final_context);

    if (is_root(MPI_COMM_WORLD)) {
        EXPECT_THAT(new_values, Eq(std::vector{0, 2, 3, 5}));
        EXPECT_THAT(local_dimensions, Eq(reshuffle::Dimensions<2>{2, 2}));
    } else {
        EXPECT_THAT(new_values, Eq(std::vector{1, 4}));
        EXPECT_THAT(local_dimensions, Eq(reshuffle::Dimensions<2>{2, 1}));
    }
}

TEST(Shuffle, CanShuffleFromBlockCyclicToBlockWise) {
    constexpr auto num_global_values = 12;
    const auto values = is_root(MPI_COMM_WORLD) ? std::vector{1, 2, 3, 4, 9, 10, 11, 12}
                                                : std::vector{5, 6, 7, 8};
    const auto num_ranks = get_num_ranks(MPI_COMM_WORLD);

    const auto initial_processor_grid = ProcessorGrid<1>{{num_ranks}};
    const auto initial_distribution = BlockCyclic{{num_global_values}, {4}, initial_processor_grid};
    const auto initial_context = Context{initial_distribution, MPI_COMM_WORLD};

    const auto final_context = create_context(DataLocationSelector::ALL_RANKS,
                                              CommSelector::ALL_RANKS, num_global_values);

    const auto new_values =
            shuffle(std::mdspan{values.data(), values.size()}, initial_context, final_context)
                    .first;

    if (is_root(MPI_COMM_WORLD)) {
        const auto expected_new_values = std::vector{1, 2, 3, 4, 5, 6};
        EXPECT_THAT(new_values, Eq(expected_new_values));
    } else {
        const auto expected_new_values = std::vector{7, 8, 9, 10, 11, 12};
        EXPECT_THAT(new_values, Eq(expected_new_values));
    }
}

TEST(Shuffle, CanShuffleFromBlockCyclicToBlockWiseIn2D) {
    const auto values = is_root(MPI_COMM_WORLD) ? std::vector{0, 2, 3, 5} : std::vector{1, 4};
    constexpr auto num_global_rows = 2;
    constexpr auto num_global_columns = 3;
    constexpr auto num_rows = 2;
    const auto num_columns = is_root(MPI_COMM_WORLD) ? 2 : 1;

    const auto comm = create_communicator(CommSelector::ALL_RANKS);

    const auto initial_distribution =
            BlockCyclic{{num_global_rows, num_global_columns}, {2, 1}, ProcessorGrid<2>{{1, 2}}};
    const auto initial_context = Context{initial_distribution, comm};

    const auto final_context =
            Context{make_block_wise_distribution({num_global_rows, num_global_columns},
                                                 ProcessorGrid<2>{{2, 1}}),
                    comm};


    const auto [new_values, local_dimensions] = shuffle(
            std::mdspan{values.data(), num_rows, num_columns}, initial_context, final_context);

    if (is_root(MPI_COMM_WORLD)) {
        EXPECT_THAT(new_values, Eq(std::vector{0, 1, 2}));
        EXPECT_THAT(local_dimensions, Eq(reshuffle::Dimensions<2>{1, 3}));
    } else {
        EXPECT_THAT(new_values, Eq(std::vector{3, 4, 5}));
        EXPECT_THAT(local_dimensions, Eq(reshuffle::Dimensions<2>{1, 3}));
    }
}

TEST(Shuffle, CanShuffleFromDifferentCommunicators) {
    constexpr auto num_values_per_rank = 6;
    const auto num_ranks = get_num_ranks(MPI_COMM_WORLD);
    const auto rank = get_rank_id(MPI_COMM_WORLD);

    const auto generator = ValuesGenerator(num_values_per_rank, num_ranks);
    const auto num_global_values = generator.get_total_num_values();

    const auto values = is_root(MPI_COMM_WORLD) ? generator.get_all_values() : std::vector<int>();

    const auto initial_context = create_context(DataLocationSelector::ONLY_RANK_0,
                                                CommSelector::ONLY_RANK_0, num_global_values);
    const auto final_context = create_context(DataLocationSelector::ALL_RANKS,
                                              CommSelector::ALL_RANKS, num_global_values);

    const auto new_values =
            shuffle(std::mdspan{values.data(), values.size()}, initial_context, final_context)
                    .first;

    EXPECT_THAT(new_values, Eq(generator.get_values_for_rank(rank)));
}

TEST(Shuffle, CanShuffleFromDifferentCommunicatorsIn2D) {
    const auto values =
            is_root(MPI_COMM_WORLD) ? std::vector{0, 1, 2, 3, 4, 5} : std::vector<int>();
    constexpr auto num_global_rows = 2;
    constexpr auto num_global_columns = 3;
    const auto num_rows = is_root(MPI_COMM_WORLD) ? num_global_rows : 0;
    const auto num_columns = is_root(MPI_COMM_WORLD) ? num_global_columns : 0;


    const auto initial_context =
            Context{make_block_wise_distribution({num_global_rows, num_global_columns},
                                                 ProcessorGrid<2>{{1, 1}}),
                    create_communicator(CommSelector::ONLY_RANK_0)};
    const auto final_context =
            Context{make_block_wise_distribution({num_global_rows, num_global_columns},
                                                 ProcessorGrid<2>{{1, 2}}),
                    create_communicator(CommSelector::ALL_RANKS)};

    const auto [new_values, local_dimensions] = shuffle(
            std::mdspan{values.data(), num_rows, num_columns}, initial_context, final_context);

    if (is_root(MPI_COMM_WORLD)) {
        EXPECT_THAT(new_values, Eq(std::vector{0, 1, 3, 4}));
        EXPECT_THAT(local_dimensions, Eq(reshuffle::Dimensions<2>{2, 2}));
    } else {
        EXPECT_THAT(new_values, Eq(std::vector{2, 5}));
        EXPECT_THAT(local_dimensions, Eq(reshuffle::Dimensions<2>{2, 1}));
    }
}

TEST(Shuffle, IfUsingTwoDifferentCommunicatorsOneMustBeSubCommunicatorOfTheOther) {
    constexpr auto num_values_per_rank = 6;
    const auto num_ranks = get_num_ranks(MPI_COMM_WORLD);

    const auto generator = ValuesGenerator(num_values_per_rank, num_ranks);
    const auto num_global_values = generator.get_total_num_values();

    const auto values = is_root(MPI_COMM_WORLD) ? generator.get_all_values() : std::vector<int>();

    const auto initial_context = create_context(DataLocationSelector::ONLY_RANK_0,
                                                CommSelector::ONLY_RANK_0, num_global_values);
    const auto final_context = create_context(DataLocationSelector::ONLY_RANK_1,
                                              CommSelector::ONLY_RANK_1, num_global_values);

    EXPECT_THROW(auto _ = shuffle(std::mdspan{values.data(), values.size()}, initial_context,
                                  final_context),
                 std::runtime_error);
}

TEST(Shuffle, IfUsingTwoDifferentCommunicatorsTheyCanStartAtDifferentRanks) {
    constexpr auto num_values_per_rank = 6;
    const auto num_ranks = get_num_ranks(MPI_COMM_WORLD);
    const auto rank = get_rank_id(MPI_COMM_WORLD);

    const auto generator = ValuesGenerator(num_values_per_rank, num_ranks);
    const auto num_global_values = generator.get_total_num_values();

    const auto values = is_root(MPI_COMM_WORLD) ? std::vector<int>() : generator.get_all_values();

    const auto initial_context = create_context(DataLocationSelector::ONLY_RANK_1,
                                                CommSelector::ONLY_RANK_1, num_global_values);
    const auto final_context = create_context(DataLocationSelector::ALL_RANKS,
                                              CommSelector::ALL_RANKS, num_global_values);

    const auto new_values =
            shuffle(std::mdspan{values.data(), values.size()}, initial_context, final_context)
                    .first;

    EXPECT_THAT(new_values, Eq(generator.get_values_for_rank(rank)));
}

auto generate_values(int from, int to) -> std::vector<int> {
    auto values_range = std::views::iota(from, to + 1);
    return {values_range.begin(), values_range.end()};
}

auto create_communicator(const CommSelector &comm_selector) -> MPI_Comm {
    switch (comm_selector) {
        case CommSelector::ONLY_RANK_0:
            return get_sub_comm(MPI_COMM_WORLD, std::vector(1, 0));
        case CommSelector::ONLY_RANK_1:
            return get_sub_comm(MPI_COMM_WORLD, std::vector(1, 1));
        case CommSelector::ALL_RANKS:
            return MPI_COMM_WORLD;
        default:
            throw std::runtime_error("Invalid CommSelector");
    }
}

auto create_context(const DataLocationSelector &data_location, const CommSelector &comm_selector,
                    const int num_global_values) -> Context<1> {
    if (is_rank_with_data_outside_comm(data_location, comm_selector)) {
        throw std::runtime_error("Want to allocate data in rank outside of communicator");
    }

    const auto comm = create_communicator(comm_selector);
    return create_context(data_location, comm, num_global_values);
}

auto create_context(const DataLocationSelector &data_location, const MPI_Comm comm,
                    const int num_global_values) -> Context<1> {
    switch (data_location) {
        case DataLocationSelector::ONLY_RANK_0:
        case DataLocationSelector::ONLY_RANK_1:
            return Context{make_block_wise_distribution({num_global_values}, ProcessorGrid<1>{{1}}),
                           comm};
        case DataLocationSelector::ALL_RANKS:
            return Context{make_block_wise_distribution({num_global_values}, ProcessorGrid<1>{{2}}),
                           comm};
        default:
            throw std::runtime_error("Invalid DataLocationSelector");
    }
}

auto is_disjoint(const DataLocationSelector &data_location, const CommSelector &comm_selector)
        -> bool {
    const auto disjoint = (comm_selector == CommSelector::ONLY_RANK_0 and
                           data_location == DataLocationSelector::ONLY_RANK_1) or
                          (comm_selector == CommSelector::ONLY_RANK_1 and
                           data_location == DataLocationSelector::ONLY_RANK_0);
    return disjoint;
}

auto is_rank_with_data_outside_comm(const DataLocationSelector &data_location,
                                    const CommSelector &comm_selector) -> bool {
    const auto data_outside_boundaries_comm = (comm_selector == CommSelector::ONLY_RANK_0 or
                                               comm_selector == CommSelector::ONLY_RANK_1) and
                                              data_location == DataLocationSelector::ALL_RANKS;
    return is_disjoint(data_location, comm_selector) or data_outside_boundaries_comm;
}
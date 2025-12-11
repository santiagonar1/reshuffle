#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <block_cyclic.hpp>
#include <block_wise.hpp>
#include <general_data_distribution.hpp>
#include <shuffle.hpp>

#include "aggregate_data.hpp"
#include "autopas_particle.hpp"
#include "complex_data.hpp"
#include "context_creation.hpp"

using namespace reshuffle;
using namespace reshuffle::mpi;

using testing::Eq;

TEST(Shuffle, CanShuffleFromOneToMany) {
    constexpr auto num_values_per_rank = 6;
    const auto num_ranks = get_num_ranks(MPI_COMM_WORLD);
    const auto rank = get_rank_id(MPI_COMM_WORLD).value();

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

TEST(Shuffle, CanShuffleCustomDatatypes) {
    constexpr auto num_global_values = 2;

    const auto values = is_root(MPI_COMM_WORLD)
                                ? std::vector{AggregateData{"one", 1}, AggregateData{"two", 2}}
                                : std::vector<AggregateData>();

    const auto initial_context = create_context(DataLocationSelector::ONLY_RANK_0,
                                                CommSelector::ALL_RANKS, num_global_values);
    const auto final_context = create_context(DataLocationSelector::ALL_RANKS,
                                              CommSelector::ALL_RANKS, num_global_values);

    const auto new_values =
            shuffle(std::mdspan{values.data(), values.size()}, initial_context, final_context)
                    .first;

    if (is_root(MPI_COMM_WORLD)) {
        EXPECT_THAT(new_values, Eq(std::vector{AggregateData{"one", 1}}));
    } else {
        EXPECT_THAT(new_values, Eq(std::vector{AggregateData{"two", 2}}));
    }
}

TEST(Shuffle, CanShuffleComplexCustomDatatypes) {
    constexpr auto num_global_values = 2;

    const auto values = is_root(MPI_COMM_WORLD) ? std::vector{Derived{0, 1}, Derived{2, 3}}
                                                : std::vector<Derived>();

    const auto initial_context = create_context(DataLocationSelector::ONLY_RANK_0,
                                                CommSelector::ALL_RANKS, num_global_values);
    const auto final_context = create_context(DataLocationSelector::ALL_RANKS,
                                              CommSelector::ALL_RANKS, num_global_values);

    const auto new_values =
            shuffle(std::mdspan{values.data(), values.size()}, initial_context, final_context)
                    .first;

    if (is_root(MPI_COMM_WORLD)) {
        EXPECT_THAT(new_values, Eq(std::vector{Derived{0, 1}}));
    } else {
        EXPECT_THAT(new_values, Eq(std::vector{Derived{2, 3}}));
    }
}

TEST(Shuffle, CanShuffleAutoPasParticles) {
    constexpr auto num_global_values = 2;

    const auto particle_to_rank_0 = MoleculeLJ{{0, 1, 2}, {3, 4, 5}, 6, 7};
    const auto particle_to_rank_1 = MoleculeLJ{{8, 9, 10}, {11, 12, 13}, 14, 15};

    const auto values = is_root(MPI_COMM_WORLD)
                                ? std::vector{particle_to_rank_0, particle_to_rank_1}
                                : std::vector<MoleculeLJ>();

    const auto initial_context = create_context(DataLocationSelector::ONLY_RANK_0,
                                                CommSelector::ALL_RANKS, num_global_values);
    const auto final_context = create_context(DataLocationSelector::ALL_RANKS,
                                              CommSelector::ALL_RANKS, num_global_values);

    const auto new_values =
            shuffle(std::mdspan{values.data(), values.size()}, initial_context, final_context)
                    .first;

    if (is_root(MPI_COMM_WORLD)) {
        EXPECT_THAT(new_values, Eq(std::vector{particle_to_rank_0}));
    } else {
        EXPECT_THAT(new_values, Eq(std::vector{particle_to_rank_1}));
    }
}

TEST(Shuffle, CanShuffleUsingGeneralDataDistribution) {
    constexpr auto num_global_values = 4;
    const auto values = is_root(MPI_COMM_WORLD) ? std::vector{2, 3} : std::vector{0, 1};
    const auto intervals_1 = std::vector{MultidimensionalInterval{Interval{0, 2}}};
    const auto intervals_0 = std::vector{MultidimensionalInterval{Interval{2, 4}}};

    const auto global_mapping = GlobalMapping<1>{{0, intervals_0}, {1, intervals_1}};

    const auto mapping_0 = std::map<IntervalId, Coordinates<1>>{{0, {0}}};
    const auto mapping_1 = std::map<IntervalId, Coordinates<1>>{{0, {0}}};

    const auto final_context = create_context(DataLocationSelector::ALL_RANKS,
                                              CommSelector::ALL_RANKS, num_global_values);

    if (is_root(MPI_COMM_WORLD)) {
        const auto initial_distribution =
                GeneralDataDistribution<1>::make(global_mapping, mapping_0, 0).value();
        const auto initial_context = Context{initial_distribution, MPI_COMM_WORLD};
        const auto new_values =
                shuffle(std::mdspan{values.data(), values.size()}, initial_context, final_context)
                        .first;

        EXPECT_THAT(new_values, Eq(std::vector{0, 1}));

    } else {
        const auto initial_distribution =
                GeneralDataDistribution<1>::make(global_mapping, mapping_1, 1).value();
        const auto initial_context = Context{initial_distribution, MPI_COMM_WORLD};
        const auto new_values =
                shuffle(std::mdspan{values.data(), values.size()}, initial_context, final_context)
                        .first;
        EXPECT_THAT(new_values, Eq(std::vector{2, 3}));
    }
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
            Context{BlockWise{{num_global_rows, num_global_columns}, ProcessorGrid{1, 1}}, comm};
    const auto final_context =
            Context{BlockWise{{num_global_rows, num_global_columns}, ProcessorGrid{1, 2}}, comm};

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
            Context{BlockWise{{num_global_rows, num_global_columns}, ProcessorGrid{1, 1}}, comm};
    const auto final_context =
            Context{BlockWise{{num_global_rows, num_global_columns}, ProcessorGrid{1, 2}}, comm};

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
            Context{BlockWise{{num_global_rows, num_global_columns}, ProcessorGrid{1, 1}}, comm};
    const auto final_context =
            Context{BlockWise{{num_global_rows, num_global_columns}, ProcessorGrid{2, 1}}, comm};

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
    const auto rank = get_rank_id(MPI_COMM_WORLD).value();

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
            Context{BlockWise{{num_global_rows, num_global_columns}, ProcessorGrid{1, 2}}, comm};
    const auto final_context =
            Context{BlockWise{{num_global_rows, num_global_columns}, ProcessorGrid{1, 1}}, comm};

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
            Context{BlockWise{{num_global_rows, num_global_columns}, ProcessorGrid{2, 1}}, comm};
    const auto final_context =
            Context{BlockWise{{num_global_rows, num_global_columns}, ProcessorGrid{1, 1}}, comm};

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
    const auto rank = get_rank_id(MPI_COMM_WORLD).value();

    const auto generator = ValuesGenerator(num_values_per_rank, num_ranks);
    const auto num_global_values = generator.get_total_num_values();

    const auto values = generator.get_values_for_rank(rank);

    const auto initial_context = create_context(DataLocationSelector::ALL_RANKS,
                                                CommSelector::ALL_RANKS, num_global_values);

    const auto final_processor_grid = ProcessorGrid{num_ranks};
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
            Context{BlockWise{{num_global_rows, num_global_columns}, ProcessorGrid{2, 1}}, comm};
    const auto final_distribution =
            BlockCyclic{{num_global_rows, num_global_columns}, {2, 1}, ProcessorGrid{1, 2}};
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

    const auto initial_processor_grid = ProcessorGrid{num_ranks};
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
            BlockCyclic{{num_global_rows, num_global_columns}, {2, 1}, ProcessorGrid{1, 2}};
    const auto initial_context = Context{initial_distribution, comm};

    const auto final_context =
            Context{BlockWise{{num_global_rows, num_global_columns}, ProcessorGrid{2, 1}}, comm};


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
    const auto rank = get_rank_id(MPI_COMM_WORLD).value();

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
            Context{BlockWise{{num_global_rows, num_global_columns}, ProcessorGrid{1, 1}},
                    create_communicator(CommSelector::ONLY_RANK_0)};
    const auto final_context =
            Context{BlockWise{{num_global_rows, num_global_columns}, ProcessorGrid{1, 2}},
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
    const auto rank = get_rank_id(MPI_COMM_WORLD).value();

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

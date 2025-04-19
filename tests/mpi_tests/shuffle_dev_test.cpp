#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <shuffle.hpp>

using namespace reshuffle::dev;

using testing::Eq;

auto is_root(const MPI_Comm &comm = MPI_COMM_WORLD) -> bool;
auto get_rank_id(const MPI_Comm &comm = MPI_COMM_WORLD) -> reshuffle::rank_id;
auto get_num_ranks(const MPI_Comm &comm = MPI_COMM_WORLD) -> int;

auto generate_values(int from, int to) -> std::vector<int>;

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

TEST(NewShuffle, CanShuffleFromOneToMany) {
    constexpr auto num_values_per_rank = 6;
    const auto num_ranks = get_num_ranks(MPI_COMM_WORLD);
    const auto rank = get_rank_id(MPI_COMM_WORLD);

    const auto generator = ValuesGenerator(num_values_per_rank, num_ranks);
    const auto num_global_values = generator.get_total_num_values();

    const auto values = is_root(MPI_COMM_WORLD) ? generator.get_all_values() : std::vector<int>();

    const auto initial_processor_grid = ProcessorGrid(1);
    const auto initial_distribution =
            make_block_wise_distribution(num_global_values, initial_processor_grid);
    const auto initial_context = Context{initial_distribution, MPI_COMM_WORLD};

    const auto final_processor_grid = ProcessorGrid(num_ranks);
    const auto final_distribution =
            make_block_wise_distribution(num_global_values, final_processor_grid);
    const auto final_context = Context{final_distribution, MPI_COMM_WORLD};

    const auto new_values = shuffle(std::span{values}, initial_context, final_context);

    EXPECT_THAT(new_values, Eq(generator.get_values_for_rank(rank)));
}

TEST(NewShuffle, CanShuffleFromManyToOne) {
    constexpr auto num_values_per_rank = 6;
    const auto num_ranks = get_num_ranks(MPI_COMM_WORLD);
    const auto rank = get_rank_id(MPI_COMM_WORLD);

    const auto generator = ValuesGenerator(num_values_per_rank, num_ranks);
    const auto num_global_values = generator.get_total_num_values();

    const auto values = generator.get_values_for_rank(rank);

    const auto initial_processor_grid = ProcessorGrid(num_ranks);
    const auto initial_distribution =
            make_block_wise_distribution(num_global_values, initial_processor_grid);
    const auto initial_context = Context{initial_distribution, MPI_COMM_WORLD};

    const auto final_processor_grid = ProcessorGrid(1);
    const auto final_distribution =
            make_block_wise_distribution(num_global_values, final_processor_grid);
    const auto final_context = Context{final_distribution, MPI_COMM_WORLD};

    const auto new_values = shuffle(std::span{values}, initial_context, final_context);

    if (is_root(MPI_COMM_WORLD)) {
        EXPECT_THAT(new_values, Eq(generator.get_all_values()));
    } else {
        EXPECT_TRUE(new_values.empty());
    }
}

TEST(NewShuffle, CanShuffleFromBlockWiseToBlockCyclic) {
    constexpr auto num_values_per_rank = 6;
    const auto num_ranks = get_num_ranks(MPI_COMM_WORLD);
    const auto rank = get_rank_id(MPI_COMM_WORLD);

    const auto generator = ValuesGenerator(num_values_per_rank, num_ranks);
    const auto num_global_values = generator.get_total_num_values();

    const auto values = generator.get_values_for_rank(rank);

    const auto initial_processor_grid = ProcessorGrid(num_ranks);
    const auto initial_distribution =
            make_block_wise_distribution(num_global_values, initial_processor_grid);
    const auto initial_context = Context{initial_distribution, MPI_COMM_WORLD};

    const auto final_processor_grid = ProcessorGrid(num_ranks);
    const auto final_distribution = BlockCyclic{num_global_values, 4, final_processor_grid};
    const auto final_context = Context{final_distribution, MPI_COMM_WORLD};

    const auto new_values = shuffle(std::span{values}, initial_context, final_context);

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

TEST(NewShuffle, CanShuffleFromBlockCyclicToBlockWise) {
    constexpr auto num_global_values = 12;
    const auto values = is_root(MPI_COMM_WORLD) ? std::vector{1, 2, 3, 4, 9, 10, 11, 12}
                                                : std::vector{5, 6, 7, 8};
    const auto num_ranks = get_num_ranks(MPI_COMM_WORLD);

    const auto initial_processor_grid = ProcessorGrid(num_ranks);
    const auto initial_distribution = BlockCyclic{num_global_values, 4, initial_processor_grid};
    const auto initial_context = Context{initial_distribution, MPI_COMM_WORLD};

    const auto final_processor_grid = ProcessorGrid(num_ranks);
    const auto final_distribution =
            make_block_wise_distribution(num_global_values, final_processor_grid);
    const auto final_context = Context{final_distribution, MPI_COMM_WORLD};

    const auto new_values = shuffle(std::span{values}, initial_context, final_context);

    if (is_root(MPI_COMM_WORLD)) {
        const auto expected_new_values = std::vector{1, 2, 3, 4, 5, 6};
        EXPECT_THAT(new_values, Eq(expected_new_values));
    } else {
        const auto expected_new_values = std::vector{7, 8, 9, 10, 11, 12};
        EXPECT_THAT(new_values, Eq(expected_new_values));
    }
}

TEST(NewShuffle, CanShuffleFromDifferentCommunicators) {
    constexpr auto num_values_per_rank = 6;
    const auto num_ranks = get_num_ranks(MPI_COMM_WORLD);
    const auto rank = get_rank_id(MPI_COMM_WORLD);

    const auto generator = ValuesGenerator(num_values_per_rank, num_ranks);
    const auto num_global_values = generator.get_total_num_values();

    const auto values = is_root(MPI_COMM_WORLD) ? generator.get_all_values() : std::vector<int>();

    const auto comm_rank_0 = reshuffle::mpi::get_sub_comm(MPI_COMM_WORLD, std::vector(1, 0));
    const auto initial_processor_grid = ProcessorGrid(1);
    const auto initial_distribution =
            make_block_wise_distribution(num_global_values, initial_processor_grid);
    const auto initial_context = Context{initial_distribution, comm_rank_0};

    const auto final_processor_grid = ProcessorGrid(num_ranks);
    const auto final_distribution =
            make_block_wise_distribution(num_global_values, final_processor_grid);
    const auto final_context = Context{final_distribution, MPI_COMM_WORLD};

    const auto new_values = shuffle(std::span{values}, initial_context, final_context);

    EXPECT_THAT(new_values, Eq(generator.get_values_for_rank(rank)));
}

TEST(NewShuffle, IfUsingTwoDifferentCommunicatorsOneMustBeSubCommunicatorOfTheOther) {
    constexpr auto num_values_per_rank = 6;
    const auto num_ranks = get_num_ranks(MPI_COMM_WORLD);
    const auto rank = get_rank_id(MPI_COMM_WORLD);

    const auto generator = ValuesGenerator(num_values_per_rank, num_ranks);
    const auto num_global_values = generator.get_total_num_values();

    const auto values = is_root(MPI_COMM_WORLD) ? generator.get_all_values() : std::vector<int>();

    const auto comm_rank_0 = reshuffle::mpi::get_sub_comm(MPI_COMM_WORLD, std::vector(1, 0));
    const auto initial_processor_grid = ProcessorGrid(1);
    const auto initial_distribution =
            make_block_wise_distribution(num_global_values, initial_processor_grid);
    const auto initial_context = Context{initial_distribution, comm_rank_0};

    const auto comm_rank_1 = reshuffle::mpi::get_sub_comm(MPI_COMM_WORLD, std::vector(1, 1));
    const auto final_processor_grid = ProcessorGrid(1);
    const auto final_distribution =
            make_block_wise_distribution(num_global_values, final_processor_grid);
    const auto final_context = Context{final_distribution, comm_rank_1};

    EXPECT_THROW(auto _ = shuffle(std::span{values}, initial_context, final_context),
                 std::runtime_error);
}


auto is_root(const MPI_Comm &comm) -> bool { return get_rank_id(comm) == 0; }

auto get_rank_id(const MPI_Comm &comm) -> reshuffle::rank_id {
    reshuffle::rank_id id{};
    MPI_Comm_rank(comm, &id);

    return id;
}

auto get_num_ranks(const MPI_Comm &comm) -> int {
    int num_ranks{};
    MPI_Comm_size(comm, &num_ranks);

    return num_ranks;
}

auto generate_values(int from, int to) -> std::vector<int> {
    auto values_range = std::views::iota(from, to + 1);
    return {values_range.begin(), values_range.end()};
}
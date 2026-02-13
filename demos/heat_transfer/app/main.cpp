#include <expected>
#include <mpi.h>

#include <reshuffle.hpp>

#include <grid_operations.hpp>
#include <processor_info.hpp>
#include <vtk_writer.hpp>

enum class GetCartesianCommError {
    INVALID_PROCESSOR_GRID,
};

enum class GetCommError {
    INVALID_NUM_RANKS,
    INVALID_COMM,
};

[[nodiscard]] auto get_processor_grid(unsigned int num_ranks) -> reshuffle::ProcessorGrid<2>;
[[nodiscard]] auto get_cartesian_comm(MPI_Comm base_comm,
                                      reshuffle::ProcessorGrid<2> processor_grid)
        -> std::expected<MPI_Comm, GetCartesianCommError>;
[[nodiscard]] auto get_comm(unsigned int num_ranks, MPI_Comm base_comm)
        -> std::expected<MPI_Comm, GetCommError>;
[[nodiscard]] auto get_next_num_ranks(int num_adaptations,
                                      const std::vector<reshuffle::RankId> &adaptation_vector)
        -> unsigned int;
[[nodiscard]] auto get_dimensions_without_ghost_layers(const heat::Grid &grid,
                                                       const heat::ProcessorInfo &processor)
        -> heat::GridDimensions;
[[nodiscard]] auto get_local_rank_grid(const heat::GridDimensions &local_dimensions,
                                       reshuffle::RankId rank) -> heat::RankGrid;
[[nodiscard]] auto scatter_from_root(const heat::Grid &global_grid,
                                     const reshuffle::Dimensions<2> &global_dimensions,
                                     unsigned int num_ranks, MPI_Comm comm) -> heat::Grid;
[[nodiscard]] auto gather_in_root(const heat::Grid &local_grid,
                                  const reshuffle::Dimensions<2> &global_dimensions, MPI_Comm comm)
        -> heat::Grid;
[[nodiscard]] auto gather_in_root(const heat::RankGrid &local_grid,
                                  const reshuffle::Dimensions<2> &global_dimensions, MPI_Comm comm)
        -> heat::RankGrid;
[[nodiscard]] auto do_adaptation(heat::Grid &local_grid, heat::RankGrid &rank_grid,
                                 const reshuffle::Dimensions<2> &global_dimensions,
                                 unsigned int new_num_ranks, MPI_Comm current_comm) -> MPI_Comm;

int main(int argc, char *argv[]) {
    constexpr auto num_iterations = 200;
    constexpr auto num_rows = 100;
    constexpr auto num_columns = num_rows;
    constexpr auto adaptation_frequency = 50;

    constexpr auto global_dimensions = reshuffle::Dimensions{num_rows, num_columns};

    MPI_Init(&argc, &argv);

    const auto num_available_ranks = reshuffle::mpi::get_num_ranks(MPI_COMM_WORLD);
    constexpr auto initial_num_ranks = 1;

    const auto processor_grid = get_processor_grid(initial_num_ranks);

    auto initial_comm =
            get_cartesian_comm(get_comm(initial_num_ranks, MPI_COMM_WORLD).value(), processor_grid)
                    .value();
    const auto initial_processor_info = heat::ProcessorInfo{initial_comm};

    const auto output_folder = std::filesystem::current_path() / "output";
    const auto files_prefix = "vtk_output_";

    const auto writer = heat::vtk::VTKWriter{output_folder, files_prefix};

    const auto global_grid = reshuffle::mpi::is_root(initial_comm)
                                     ? heat::initialize_grid(num_rows, num_columns)
                                     : heat::Grid{};

    auto local_grid = heat::add_ghost_layers(
            scatter_from_root(global_grid, global_dimensions, initial_num_ranks, initial_comm),
            initial_processor_info);

    auto local_rank_grid = get_local_rank_grid(
            get_dimensions_without_ghost_layers(local_grid, initial_processor_info),
            initial_processor_info.get_rank());

    auto rank_grid = gather_in_root(local_rank_grid, global_dimensions, initial_comm);

    auto current_comm = initial_comm;
    const auto adaptation_vector = std::views::iota(1) | std::views::take(num_available_ranks) |
                                   std::ranges::to<std::vector<reshuffle::RankId>>();
    auto num_adaptations = 0;
    for (auto i = 0; i < num_iterations; i++) {
        // Simplification: we only do adaptations inside the computational loop
        if (i % adaptation_frequency == 0) {
            MPI_Barrier(MPI_COMM_WORLD);

            const auto new_num_ranks = get_next_num_ranks(num_adaptations, adaptation_vector);

            const auto new_comm = do_adaptation(local_grid, rank_grid, global_dimensions,
                                                new_num_ranks, current_comm);

            if (current_comm != initial_comm and current_comm != MPI_COMM_NULL) {
                MPI_Comm_free(&current_comm);
            }

            current_comm = new_comm;
            num_adaptations++;
        }


        if (reshuffle::mpi::belongs_to_comm(current_comm)) {
            local_grid = heat::exchange_ghost_layers(local_grid, current_comm);

            const auto current_processor_info = heat::ProcessorInfo{current_comm};
            writer.record_timestep(i, gather_in_root(local_grid, global_dimensions, current_comm),
                                   current_processor_info.get_rank(), rank_grid);

            local_grid = heat::apply_jacobi(local_grid);
        }
    }

    if (initial_comm != MPI_COMM_NULL) { MPI_Comm_free(&initial_comm); }
    MPI_Finalize();
    return 0;
}

auto get_processor_grid(const unsigned int num_ranks) -> reshuffle::ProcessorGrid<2> {
    constexpr auto num_dimensions = 2;

    auto dimensions = reshuffle::Dimensions<num_dimensions>{};
    MPI_Dims_create(static_cast<int>(num_ranks), num_dimensions, dimensions.data());

    return reshuffle::ProcessorGrid{dimensions};
}

auto get_cartesian_comm(const MPI_Comm base_comm, const reshuffle::ProcessorGrid<2> processor_grid)
        -> std::expected<MPI_Comm, GetCartesianCommError> {
    constexpr auto num_dimensions = 2;

    if (not reshuffle::mpi::belongs_to_comm(base_comm)) { return MPI_COMM_NULL; }

    if (const auto num_available_ranks = reshuffle::mpi::get_num_ranks(base_comm);
        processor_grid.get_num_processors() != num_available_ranks) {
        return std::unexpected(GetCartesianCommError::INVALID_PROCESSOR_GRID);
    }

    constexpr auto periods = std::array<int, num_dimensions>{};
    constexpr auto reorder = true;
    MPI_Comm cartesian_comm;
    MPI_Cart_create(base_comm, num_dimensions, processor_grid.get_dimensions().data(),
                    periods.data(), reorder, &cartesian_comm);

    return cartesian_comm;
}

auto get_comm(const unsigned int num_ranks, const MPI_Comm base_comm)
        -> std::expected<MPI_Comm, GetCommError> {
    if (num_ranks == 0) { return std::unexpected(GetCommError::INVALID_NUM_RANKS); }

    if (base_comm == MPI_COMM_NULL) { return std::unexpected(GetCommError::INVALID_COMM); }

    if (const auto available_ranks = reshuffle::mpi::get_num_ranks(base_comm);
        num_ranks > available_ranks) {
        return std::unexpected(GetCommError::INVALID_NUM_RANKS);
    }

    auto ranks = std::vector<int>(num_ranks);
    std::ranges::iota(ranks.begin(), ranks.end(), 0);

    return reshuffle::mpi::get_sub_comm(base_comm, ranks);
}

auto get_next_num_ranks(const int num_adaptations,
                        const std::vector<reshuffle::RankId> &adaptation_vector) -> unsigned int {
    const auto num_possible_values = adaptation_vector.size();
    auto next_num_ranks = adaptation_vector[num_adaptations % num_possible_values];


    return next_num_ranks;
}

auto get_dimensions_without_ghost_layers(const heat::Grid &grid,
                                         const heat::ProcessorInfo &processor)
        -> heat::GridDimensions {
    const auto dimensions = heat::get_dimensions(grid);

    if (dimensions.num_rows == 0 or dimensions.num_columns == 0) { return {}; }

    return heat::GridDimensions{dimensions.num_rows - (processor.has_up_neighbour() ? 1 : 0) -
                                        (processor.has_down_neighbour() ? 1 : 0),
                                dimensions.num_columns - (processor.has_left_neighbour() ? 1 : 0) -
                                        (processor.has_right_neighbour() ? 1 : 0)};
}

auto get_local_rank_grid(const heat::GridDimensions &local_dimensions, const reshuffle::RankId rank)
        -> heat::RankGrid {
    if (local_dimensions.num_rows == 0 or local_dimensions.num_columns == 0) { return {}; }

    return {local_dimensions.num_rows, std::vector(local_dimensions.num_columns, rank)};
}

auto scatter_from_root(const heat::Grid &global_grid,
                       const reshuffle::Dimensions<2> &global_dimensions,
                       const unsigned int num_ranks, MPI_Comm comm) -> heat::Grid {

    const auto processor_grid = get_processor_grid(num_ranks);

    const auto all_in_root = reshuffle::Context{
            reshuffle::BlockWise{global_dimensions, reshuffle::ProcessorGrid{1, 1}}, comm};
    const auto final_context =
            reshuffle::Context{reshuffle::BlockWise{global_dimensions, processor_grid}, comm};

    auto local_grid = heat::to_grid(reshuffle::shuffle(global_grid, all_in_root, final_context))
                              .value_or(heat::Grid{});

    return local_grid;
}

auto gather_in_root(const heat::Grid &local_grid, const reshuffle::Dimensions<2> &global_dimensions,
                    const MPI_Comm comm) -> heat::Grid {
    const auto processor = heat::ProcessorInfo{comm};

    const auto current_num_ranks = reshuffle::mpi::get_num_ranks(comm);
    const auto current_processor_grid = get_processor_grid(current_num_ranks);
    const auto current_context = reshuffle::Context{
            reshuffle::BlockWise{global_dimensions, current_processor_grid}, comm};

    const auto all_in_rank_0 = reshuffle::Context{
            reshuffle::BlockWise{global_dimensions, reshuffle::ProcessorGrid{1, 1}}, comm};

    const auto global_grid =
            heat::to_grid(reshuffle::shuffle(heat::remove_ghost_layers(local_grid, processor),
                                             current_context, all_in_rank_0))
                    .value();

    return global_grid;
}

auto gather_in_root(const heat::RankGrid &local_grid,
                    const reshuffle::Dimensions<2> &global_dimensions, const MPI_Comm comm)
        -> heat::RankGrid {
    if (not reshuffle::mpi::belongs_to_comm(comm)) { return {}; }

    const auto current_num_ranks = reshuffle::mpi::get_num_ranks(comm);
    const auto current_processor_grid = get_processor_grid(current_num_ranks);
    const auto current_context = reshuffle::Context{
            reshuffle::BlockWise{global_dimensions, current_processor_grid}, comm};

    const auto all_in_rank_0 = reshuffle::Context{
            reshuffle::BlockWise{global_dimensions, reshuffle::ProcessorGrid{1, 1}}, comm};

    const auto global_grid =
            heat::to_grid(reshuffle::shuffle(local_grid, current_context, all_in_rank_0)).value();

    return global_grid;
}

auto do_adaptation(heat::Grid &local_grid, heat::RankGrid &rank_grid,
                   const reshuffle::Dimensions<2> &global_dimensions,
                   const unsigned int new_num_ranks, MPI_Comm current_comm) -> MPI_Comm {

    auto current_num_ranks = reshuffle::mpi::belongs_to_comm(current_comm)
                                     ? reshuffle::mpi::get_num_ranks(current_comm)
                                     : 1;

    MPI_Bcast(&current_num_ranks, 1, MPI_INT, 0, MPI_COMM_WORLD);

    const auto current_processor_grid = get_processor_grid(current_num_ranks);
    const auto new_processor_grid = get_processor_grid(new_num_ranks);

    auto new_comm =
            get_cartesian_comm(get_comm(new_num_ranks, MPI_COMM_WORLD).value(), new_processor_grid)
                    .value();

    const auto current_context = reshuffle::Context{
            reshuffle::BlockWise{global_dimensions, current_processor_grid}, current_comm};

    const auto new_context = reshuffle::Context{
            reshuffle::BlockWise{global_dimensions, new_processor_grid}, new_comm};

    const auto current_processor_info = heat::ProcessorInfo{current_comm};
    const auto new_processor_info = heat::ProcessorInfo{new_comm};

    local_grid = heat::add_ghost_layers(
            heat::to_grid(reshuffle::shuffle(
                                  heat::remove_ghost_layers(local_grid, current_processor_info),
                                  current_context, new_context))
                    .value_or(heat::Grid{}),
            new_processor_info);

    rank_grid = gather_in_root(
            get_local_rank_grid(get_dimensions_without_ghost_layers(local_grid, new_processor_info),
                                new_processor_info.get_rank()),
            global_dimensions, new_comm);

    return new_comm;
}
#include <benchmark/benchmark.h>
#include <chrono>

#include <greedy_rank_order_strategy.hpp>
#include <hungarian_rank_order_strategy.hpp>
#include <rank_order.hpp>


void rank_order_strategy_greedy(benchmark::State &state) {
    const auto matrix_size = static_cast<int>(state.range(0));

    // Create a representative communication matrix with varied weights
    reshuffle::internal::Matrix2D<int> communication_matrix(matrix_size);
    for (int i = 0; i < matrix_size; ++i) {
        communication_matrix[i].resize(matrix_size);
        for (int j = 0; j < matrix_size; ++j) {
            // Higher values on diagonal, but significant off-diagonal communication
            if (i == j) {
                communication_matrix[i][j] = 100 + (i * j) % 50;
            } else {
                communication_matrix[i][j] = (i + j) % 30 + 1;
            }
        }
    }

    const auto greedy_strategy = reshuffle::internal::GreedyRankOrderStrategy{};

    for (auto _: state) {
        auto result = greedy_strategy.get_optimal_order(communication_matrix);
        benchmark::DoNotOptimize(result);
    }
}

void rank_order_strategy_hungarian(benchmark::State &state) {
    const auto matrix_size = static_cast<int>(state.range(0));

    // Create the same communication matrix as for greedy
    reshuffle::internal::Matrix2D<int> communication_matrix(matrix_size);
    for (int i = 0; i < matrix_size; ++i) {
        communication_matrix[i].resize(matrix_size);
        for (int j = 0; j < matrix_size; ++j) {
            if (i == j) {
                communication_matrix[i][j] = 100 + (i * j) % 50;
            } else {
                communication_matrix[i][j] = (i + j) % 30 + 1;
            }
        }
    }

    const auto hungarian_strategy = reshuffle::internal::HungarianRankOrderStrategy{};

    for (auto _: state) {
        auto result = hungarian_strategy.get_optimal_order(communication_matrix);
        benchmark::DoNotOptimize(result);
    }
}

void rank_order_strategy_greedy_sparse(benchmark::State &state) {
    const auto matrix_size = static_cast<int>(state.range(0));

    // Create a sparse communication matrix (clustered communication pattern)
    reshuffle::internal::Matrix2D<int> communication_matrix(matrix_size);
    for (int i = 0; i < matrix_size; ++i) {
        communication_matrix[i].resize(matrix_size);
        for (int j = 0; j < matrix_size; ++j) {
            if (std::abs(i - j) <= 2) {
                communication_matrix[i][j] = 50 + (i * j) % 100;
            } else {
                communication_matrix[i][j] = (i + j) % 5;
            }
        }
    }

    const auto greedy_strategy = reshuffle::internal::GreedyRankOrderStrategy{};

    for (auto _: state) {
        auto result = greedy_strategy.get_optimal_order(communication_matrix);
        benchmark::DoNotOptimize(result);
    }
}

void rank_order_strategy_hungarian_sparse(benchmark::State &state) {
    const auto matrix_size = static_cast<int>(state.range(0));

    // Create the same sparse communication matrix as for greedy
    reshuffle::internal::Matrix2D<int> communication_matrix(matrix_size);
    for (int i = 0; i < matrix_size; ++i) {
        communication_matrix[i].resize(matrix_size);
        for (int j = 0; j < matrix_size; ++j) {
            if (std::abs(i - j) <= 2) {
                communication_matrix[i][j] = 50 + (i * j) % 100;
            } else {
                communication_matrix[i][j] = (i + j) % 5;
            }
        }
    }

    const auto hungarian_strategy = reshuffle::internal::HungarianRankOrderStrategy{};

    for (auto _: state) {
        auto result = hungarian_strategy.get_optimal_order(communication_matrix);
        benchmark::DoNotOptimize(result);
    }
}

void rank_order_strategy_greedy_skewed(benchmark::State &state) {
    const auto matrix_size = static_cast<int>(state.range(0));

    // Create a highly skewed communication matrix
    reshuffle::internal::Matrix2D<int> communication_matrix(matrix_size);
    for (int i = 0; i < matrix_size; ++i) {
        communication_matrix[i].resize(matrix_size);
        for (int j = 0; j < matrix_size; ++j) {
            // The first quarter of ranks communicate heavily with each other
            if (i < matrix_size / 4 && j < matrix_size / 4) {
                communication_matrix[i][j] = 200 + (i * j) % 100;
            } else if (i < matrix_size / 4 || j < matrix_size / 4) {
                communication_matrix[i][j] = 50 + (i + j) % 50;
            } else {
                communication_matrix[i][j] = (i + j) % 10;
            }
        }
    }

    const auto greedy_strategy = reshuffle::internal::GreedyRankOrderStrategy{};

    for (auto _: state) {
        auto result = greedy_strategy.get_optimal_order(communication_matrix);
        benchmark::DoNotOptimize(result);
    }
}

void rank_order_strategy_hungarian_skewed(benchmark::State &state) {
    const auto matrix_size = static_cast<int>(state.range(0));

    // Create the same skewed communication matrix as for greedy
    reshuffle::internal::Matrix2D<int> communication_matrix(matrix_size);
    for (int i = 0; i < matrix_size; ++i) {
        communication_matrix[i].resize(matrix_size);
        for (int j = 0; j < matrix_size; ++j) {
            if (i < matrix_size / 4 && j < matrix_size / 4) {
                communication_matrix[i][j] = 200 + (i * j) % 100;
            } else if (i < matrix_size / 4 || j < matrix_size / 4) {
                communication_matrix[i][j] = 50 + (i + j) % 50;
            } else {
                communication_matrix[i][j] = (i + j) % 10;
            }
        }
    }

    const auto hungarian_strategy = reshuffle::internal::HungarianRankOrderStrategy{};

    for (auto _: state) {
        auto result = hungarian_strategy.get_optimal_order(communication_matrix);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(rank_order_strategy_hungarian)
        ->RangeMultiplier(2)
        ->Range(8, 512)
        ->Unit(benchmark::kMicrosecond)
        ->Repetitions(5);


BENCHMARK(rank_order_strategy_greedy)
        ->RangeMultiplier(2)
        ->Range(8, 512)
        ->Unit(benchmark::kMicrosecond)
        ->Repetitions(5);


BENCHMARK(rank_order_strategy_hungarian_sparse)
        ->RangeMultiplier(2)
        ->Range(8, 512)
        ->Unit(benchmark::kMicrosecond)
        ->Repetitions(5);


BENCHMARK(rank_order_strategy_greedy_sparse)
        ->RangeMultiplier(2)
        ->Range(8, 512)
        ->Unit(benchmark::kMicrosecond)
        ->Repetitions(5);


BENCHMARK(rank_order_strategy_hungarian_skewed)
        ->RangeMultiplier(2)
        ->Range(8, 512)
        ->Unit(benchmark::kMicrosecond)
        ->Repetitions(5);


BENCHMARK(rank_order_strategy_greedy_skewed)
        ->RangeMultiplier(2)
        ->Range(8, 512)
        ->Unit(benchmark::kMicrosecond)
        ->Repetitions(5);

int main(int argc, char **argv) {
    benchmark::Initialize(&argc, argv);
    benchmark::RunSpecifiedBenchmarks();
    return 0;
}
import matplotlib.pyplot as plt
import argparse

from common import utils


def plot_comparison(base_metrics, compare_metrics, metric_name: str, experiment: str):
    plt.plot(base_metrics[experiment]["num_elements"], base_metrics[experiment][metric_name], label=f"Base")
    plt.plot(compare_metrics[experiment]["num_elements"], compare_metrics[experiment][metric_name],
             label=f"Contender")

    plt.title(f"{experiment}")
    plt.xlabel("Number of Elements")
    plt.ylabel("Value")
    plt.legend()
    plt.show()


def get_common_experiments(base_metrics, contender_metrics):
    return set(base_metrics.keys()).intersection(contender_metrics.keys())


def get_args():
    parser = argparse.ArgumentParser(description="Plot benchmark metrics.")

    parser.add_argument(
        "--base_file",
        type=str,
        required=True,
        help="Path to the JSON file with benchmark results"
    )

    parser.add_argument(
        "--contender_file",
        type=str,
        required=True,
        help="Compare the results of the base benchmark with the results of this benchmark"
    )

    parser.add_argument(
        "--metrics",
        type=str,
        nargs='*',
        default=["cpu_time", "real_time"],
        help="List of metric names to plot, e.g., --metrics cpu_time real_time"
    )

    parser.add_argument(
        "--benchmark_filter",
        required=False,
        type=str,
        help="Show the results of the benchmarks that contain the given string in their name"
    )

    return parser.parse_args()


def main():
    args = get_args()

    metric_names = args.metrics
    base_file = args.base_file
    benchmark_filter = args.benchmark_filter
    contender_file = args.contender_file

    base_results = utils.load_benchmark_results(base_file, benchmark_filter)
    base_metrics = utils.get_metrics(base_results, metric_names)

    contender_results = utils.load_benchmark_results(contender_file, benchmark_filter)
    contender_metrics = utils.get_metrics(contender_results, metric_names)

    common_experiments = get_common_experiments(base_metrics, contender_metrics)

    for metric_name in metric_names:
        for experiment in common_experiments:
            plot_comparison(base_metrics, contender_metrics, metric_name, experiment)


if __name__ == "__main__":
    main()

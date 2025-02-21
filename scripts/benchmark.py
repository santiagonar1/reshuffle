import matplotlib.pyplot as plt
import argparse


from common import utils


def plot_metrics(metrics, metric_name: str):
    for experiment, metric_data in metrics.items():
        plt.plot(metric_data["num_elements"], metric_data[metric_name], label=f"{experiment}")

    plt.title(f"{metric_name}")
    plt.xlabel("Number of Elements")
    plt.ylabel("Value")
    plt.legend()
    plt.show()


def get_args():
    parser = argparse.ArgumentParser(description="Plot benchmark metrics.")

    parser.add_argument(
        "--metrics",
        type=str,
        nargs='*',
        default=["cpu_time", "real_time"],
        help="List of metric names to plot, e.g., --metrics cpu_time real_time"
    )

    parser.add_argument(
        "--result_file",
        type=str,
        default="benchmark_example/results.json",
        help="Path to the JSON file with benchmark results"
    )

    parser.add_argument(
        "--benchmark_filter",
        required=False,
        type=str,
        help="Show the results of the benchmarks that contain the given string in their name"
    )

    parser.add_argument(
        "--compare_with",
        type=str,
        required=False,
        help="Compare the results of the current benchmark with the results of the given benchmark"
    )

    return parser.parse_args()


def main():
    args = get_args()

    metric_names = args.metrics
    results_fname = args.result_file
    benchmark_filter = args.benchmark_filter
    compare_with = args.compare_with

    results = utils.load_benchmark_results(results_fname, benchmark_filter)
    metrics = utils.get_metrics(results, metric_names)

    for metric_name in metric_names:
        plot_metrics(metrics, metric_name)

    if compare_with:
        compare_results = utils.load_benchmark_results(compare_with, benchmark_filter)
        compare_metrics = utils.get_metrics(compare_results, metric_names)

        for metric_name in metric_names:
            for experiment, metric_data in metrics.items():
                plt.plot(metric_data["num_elements"], metric_data[metric_name], label=f"{results_fname}")
                plt.plot(compare_metrics[experiment]["num_elements"], compare_metrics[experiment][metric_name], label=f"{compare_with}")

                plt.title(f"{experiment}")
                plt.xlabel("Number of Elements")
                plt.ylabel("Value")
                plt.legend()
                plt.show()


if __name__ == "__main__":
    main()

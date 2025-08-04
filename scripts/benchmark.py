import matplotlib.pyplot as plt
import argparse


from common import utils


def plot_metrics(metrics, metric_name: str, title: str):
    for experiment, metric_data in metrics.items():
        plt.plot(metric_data["num_elements"], metric_data[metric_name], label=f"{experiment}")

    plt.title(f"{title}")
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

    return parser.parse_args()


def main():
    args = get_args()

    metric_names = args.metrics
    results_file = args.result_file
    benchmark_filter = args.benchmark_filter

    results = utils.load_benchmark_results(results_file, benchmark_filter)
    metrics = utils.get_metrics(results, metric_names)
    time_unit =  results[0]["time_unit"] # Let's assume all of them have the same time unit

    for metric_name in metric_names:
        plot_metrics(metrics, metric_name, f"{metric_name} ({time_unit})")


if __name__ == "__main__":
    main()

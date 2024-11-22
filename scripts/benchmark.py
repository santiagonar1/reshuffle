import json
import matplotlib.pyplot as plt


def load_benchmark_results(fname: str):
    with open(fname, "r") as f:
        results = json.load(f)
        return results["benchmarks"]


def get_experiment_names(results) -> [str]:
    experiment_names = set()
    for result in results:
        name, _, _ = (result.get("run_name", None) or result["name"]).split("/")
        experiment_names.add(name)
    return list(experiment_names)


def get_metrics(results, metric_names: [str]):
    experiment_names: [str] = get_experiment_names(results)
    metrics = {experiment: {metric_name: [] for metric_name in metric_names + ["num_elements"]} for experiment in
               experiment_names}

    for result in results:
        name, num_elements, _ = (result.get("run_name", None) or result["name"]).split("/")
        metrics[name]["num_elements"].append(int(num_elements))
        for metric_name in metric_names:
            metrics[name][metric_name].append(float(result[metric_name]))

    return metrics


def plot_metrics(metrics, metric_name: str):
    for experiment, metric_data in metrics.items():
        plt.plot(metric_data["num_elements"], metric_data[metric_name], label=f"{experiment}")

    plt.title(f"{metric_name}")
    plt.xlabel("Number of Elements")
    plt.ylabel("Value")
    plt.legend()
    plt.show()


def main():
    results = load_benchmark_results("benchmark_example/results.json")
    metrics = get_metrics(results, ["cpu_time", "real_time"])
    plot_metrics(metrics, "cpu_time")
    plot_metrics(metrics, "real_time")


if __name__ == "__main__":
    main()

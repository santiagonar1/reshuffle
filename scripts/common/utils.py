import json
import re


def load_benchmark_results(fname: str, benchmark_filter: str) -> [dict]:
    def benchmark_wanted(benchmark):
        if benchmark_filter is None:
            return True
        name = benchmark.get("run_name", None) or benchmark["name"]
        return re.search(benchmark_filter, name) is not None

    with open(fname, "r") as f:
        results = json.load(f)
        return list(filter(benchmark_wanted, results["benchmarks"]))


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
import argparse
import pathlib
import sys
import time

# Allow direct execution from the source tree after an in-place extension build.
sys.path.insert(0, str(pathlib.Path(__file__).resolve().parents[1]))

from pq_experiments import (
    RankPredictionPriorityQueue,
    aggregate_rank_stats,
    build_key_rank_predictor,
    build_node_rank_predictor,
    dijkstra_with_trace,
    key_rank_error_stats,
    load_dimacs_graph,
    node_rank_error_stats,
    rank_prediction_stats,
    true_ranks,
    write_push_trace_csv,
    write_summary_json,
)


IMPLEMENTATIONS = (
    "binary_heap",
    "fibonacci_heap",
    "kaplan_heap",
)
DEFAULT_GRAPH = pathlib.Path("graphs/dimacs/USA-road-d.NY.gr")
DEFAULT_MAX_ARCS = 50000
DEFAULT_RESULT_DIR = pathlib.Path("results")


def drain(queue):
    values = []
    while queue:
        values.append(queue.pop())
    return values


def test_true_ranks_are_one_based_and_distinct():
    assert true_ranks([7, 3, 9, 1]) == {
        1: 1,
        3: 2,
        7: 3,
        9: 4,
    }

    try:
        true_ranks([1, 1, 2])
    except ValueError:
        return

    raise AssertionError("duplicate keys were accepted in a rank fixture")


def test_rank_prediction_stats():
    assert rank_prediction_stats([(1, 1), (2, 4), (5, 3)]) == {
        "count": 3,
        "total_error": 4,
        "max_error": 2,
        "average_error": 4 / 3,
    }
    assert rank_prediction_stats([]) == {
        "count": 0,
        "total_error": 0,
        "max_error": None,
        "average_error": None,
    }


def test_rank_predictions_preserve_queue_order(implementation):
    values = [7, 3, 9, 1, 4, 8, 2]
    ranks = true_ranks(values)
    offsets = {
        1: 0,
        2: 1,
        3: -1,
        4: 2,
        7: -2,
        8: 1,
        9: 0,
    }
    queue = RankPredictionPriorityQueue(implementation=implementation)

    for value in values:
        queue.push(
            value,
            predicted_rank=ranks[value] + offsets[value],
            true_rank=ranks[value],
        )

    assert queue.peek() == 1
    assert drain(queue) == sorted(values)

    stats = queue.stats()
    assert stats["insertions"] == len(values)
    assert stats["size"] == 0
    assert stats["rank_prediction_error_count"] == len(values)
    assert stats["max_rank_prediction_error"] == 2
    assert stats["average_rank_prediction_error"] == 1


def test_rank_predictions_with_explicit_priorities():
    items = [
        ("slow", 30),
        ("fast", 5),
        ("middle", 20),
    ]
    ranks = true_ranks(items, key=lambda item: item[1])
    queue = RankPredictionPriorityQueue(
        key=lambda item: item[1],
        implementation="binary_heap",
    )

    for item in items:
        queue.push(
            item,
            predicted_rank=ranks[item],
            true_rank=ranks[item],
        )

    assert [name for name, _priority in drain(queue)] == [
        "fast",
        "middle",
        "slow",
    ]


def test_aggregate_rank_stats():
    aggregate = aggregate_rank_stats([
        {"count": 2, "total_error": 5, "max_error": 3, "average_error": 2.5},
        {"count": 3, "total_error": 6, "max_error": 4, "average_error": 2.0},
    ])

    assert aggregate == {
        "count": 5,
        "total_error": 11,
        "max_error": 4,
        "average_error": 2.2,
    }


def checksum_distances(distances, node_count):
    checksum = 1469598103934665603
    mask = (1 << 64) - 1
    inf = (1 << 64) - 1

    for node in range(1, node_count + 1):
        value = int(distances.get(node, inf))
        checksum ^= (value + node * 1099511628211) & mask
        checksum = (checksum * 1099511628211) & mask

    return checksum


def default_sources(graph):
    active_nodes = [
        node
        for node in range(1, graph.node_count + 1)
        if graph.offsets[node] < graph.offsets[node + 1]
    ]
    if not active_nodes:
        return [1]

    return sorted({
        active_nodes[0],
        active_nodes[len(active_nodes) // 3],
        active_nodes[(2 * len(active_nodes)) // 3],
    })


def run_graph_rank_prediction_experiment(
    graph_path,
    *,
    sources=None,
    implementations=IMPLEMENTATIONS,
    max_arcs=DEFAULT_MAX_ARCS,
    result_dir=DEFAULT_RESULT_DIR,
):
    graph = load_dimacs_graph(graph_path, max_arcs=max_arcs)
    if sources is None:
        sources = default_sources(graph)

    all_rows = []
    runs = []
    expected_checksum_by_source = {}
    previous_by_implementation = {
        implementation: None for implementation in implementations
    }

    for implementation in implementations:
        for run_index, source in enumerate(sources, start=1):
            previous = previous_by_implementation[implementation]
            node_rank_predictor = None
            key_rank_predictor = None
            if previous is not None:
                node_rank_predictor = build_node_rank_predictor(
                    previous["extraction_order"]
                )
                key_rank_predictor = build_key_rank_predictor(
                    previous["inserted_keys"]
                )

            start = time.perf_counter()
            trace = dijkstra_with_trace(
                graph,
                source,
                implementation=implementation,
                node_rank_predictor=node_rank_predictor,
                key_rank_predictor=key_rank_predictor,
            )
            seconds = time.perf_counter() - start
            checksum = checksum_distances(trace["distances"], graph.node_count)

            expected = expected_checksum_by_source.setdefault(source, checksum)
            if checksum != expected:
                raise AssertionError(
                    f"distance checksum changed for {implementation} "
                    f"source {source}"
                )

            node_stats = node_rank_error_stats(trace)
            key_stats = key_rank_error_stats(trace["push_trace"])
            runs.append({
                "run": run_index,
                "implementation": implementation,
                "source": source,
                "seconds": seconds,
                "checksum": checksum,
                "stats": trace["stats"],
                "node_rank_prediction": node_stats,
                "key_rank_prediction": key_stats,
            })

            for event in trace["push_trace"]:
                row = dict(event)
                row["run"] = run_index
                row["implementation"] = implementation
                row["source"] = source
                all_rows.append(row)

            previous_by_implementation[implementation] = trace

    node_stats = aggregate_rank_stats(
        run["node_rank_prediction"] for run in runs
    )
    key_stats = aggregate_rank_stats(
        run["key_rank_prediction"] for run in runs
    )
    result_stem = graph_path.stem
    if max_arcs is not None:
        result_stem = f"{result_stem}-first-{max_arcs}-arcs"

    summary_path = result_dir / f"{result_stem}-augmented-summary.json"
    trace_path = result_dir / f"{result_stem}-augmented-push-trace.csv"
    summary = {
        "graph": str(graph_path),
        "node_count": graph.node_count,
        "arc_count": graph.arc_count,
        "loaded_arc_count": graph.loaded_arc_count,
        "max_arcs": max_arcs,
        "sources": list(sources),
        "implementations": list(implementations),
        "model": "rank predictions from the previous Dijkstra run",
        "runs": runs,
        "aggregate_node_rank_prediction": node_stats,
        "aggregate_key_rank_prediction": key_stats,
        "trace_csv": str(trace_path),
    }

    write_summary_json(summary_path, summary)
    write_push_trace_csv(trace_path, all_rows)

    return summary_path, trace_path, summary


def parse_sources(raw_sources):
    if raw_sources is None:
        return None
    return [int(source) for source in raw_sources.split(",") if source]


def parse_args():
    parser = argparse.ArgumentParser(
        description=(
            "Run rank-prediction Dijkstra experiments for hpqlib's Python "
            "learning-augmented prototype."
        )
    )
    parser.add_argument(
        "graph",
        nargs="?",
        default=DEFAULT_GRAPH,
        type=pathlib.Path,
        help="DIMACS .gr graph path",
    )
    parser.add_argument(
        "--sources",
        help="comma-separated one-based source nodes; defaults to three nodes",
    )
    parser.add_argument(
        "--max-arcs",
        default=DEFAULT_MAX_ARCS,
        type=int,
        help=(
            "maximum number of arcs to load; use 0 to load the whole graph"
        ),
    )
    parser.add_argument(
        "--result-dir",
        default=DEFAULT_RESULT_DIR,
        type=pathlib.Path,
        help="directory for generated JSON and CSV outputs",
    )
    return parser.parse_args()


def main():
    args = parse_args()
    test_true_ranks_are_one_based_and_distinct()
    test_rank_prediction_stats()
    for implementation in IMPLEMENTATIONS:
        test_rank_predictions_preserve_queue_order(implementation)
    test_rank_predictions_with_explicit_priorities()
    test_aggregate_rank_stats()

    max_arcs = args.max_arcs
    if max_arcs == 0:
        max_arcs = None
    summary_path, trace_path, summary = run_graph_rank_prediction_experiment(
        args.graph,
        sources=parse_sources(args.sources),
        max_arcs=max_arcs,
        result_dir=args.result_dir,
    )
    print(
        "All augmented priority_queue tests passed; "
        f"wrote {summary_path} and {trace_path} "
        f"({len(summary['runs'])} Dijkstra runs)"
    )


if __name__ == "__main__":
    main()

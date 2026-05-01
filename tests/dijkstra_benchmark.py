import argparse
import pathlib
import sys
import time

# Allow direct execution from the source tree after an in-place extension build.
sys.path.insert(0, str(pathlib.Path(__file__).resolve().parents[1]))

from pq_experiments import dijkstra_with_trace, load_dimacs_graph


IMPLEMENTATIONS = (
    "binary_heap",
    "fibonacci_heap",
    "kaplan_heap",
)
DEFAULT_GRAPH = pathlib.Path("graphs/dimacs/USA-road-d.NY.gr")


def checksum_distances(distances, node_count):
    checksum = 1469598103934665603
    mask = (1 << 64) - 1
    inf = (1 << 64) - 1

    for node in range(1, node_count + 1):
        value = int(distances.get(node, inf))
        checksum ^= (value + node * 1099511628211) & mask
        checksum = (checksum * 1099511628211) & mask

    return checksum


def run_dijkstra_benchmark(graph_path, source):
    graph = load_dimacs_graph(graph_path)
    expected_checksum = None
    expected_reachable = None

    print(
        f"graph nodes={graph.node_count} arcs={graph.arc_count} "
        f"loaded_arcs={graph.loaded_arc_count} source={source}"
    )
    print(
        f"{'implementation':<16} {'seconds':>12} {'reachable':>12} "
        f"{'pushes':>12} {'stale':>12} {'checksum':>18}"
    )

    for implementation in IMPLEMENTATIONS:
        start = time.perf_counter()
        trace = dijkstra_with_trace(graph, source, implementation=implementation)
        seconds = time.perf_counter() - start
        reachable = len(trace["distances"])
        checksum = checksum_distances(trace["distances"], graph.node_count)

        if expected_checksum is None:
            expected_checksum = checksum
            expected_reachable = reachable
        elif checksum != expected_checksum or reachable != expected_reachable:
            raise AssertionError(f"result mismatch for {implementation}")

        print(
            f"{implementation:<16} {seconds:12.6f} {reachable:12d} "
            f"{trace['stats']['pushes']:12d} {trace['stats']['stale_pops']:12d} "
            f"{checksum:18d}"
        )


def parse_args():
    parser = argparse.ArgumentParser(
        description="Run Dijkstra with each Python hpqlib heap backend."
    )
    parser.add_argument(
        "graph",
        nargs="?",
        default=DEFAULT_GRAPH,
        type=pathlib.Path,
        help="DIMACS .gr graph path",
    )
    parser.add_argument(
        "source",
        nargs="?",
        default=1,
        type=int,
        help="one-based source node",
    )
    return parser.parse_args()


def main():
    args = parse_args()
    run_dijkstra_benchmark(args.graph, args.source)


if __name__ == "__main__":
    main()

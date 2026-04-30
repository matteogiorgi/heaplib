# tests

This directory contains automatic checks for hpqlib and Python-side experimental
code used by those checks.

Tests should make failures visible.  They should cover expected behavior, edge
cases, regressions, and integration points such as the CPython binding.  Code in
this directory may use assertions, fixtures, helper prototypes, or small
testing-only abstractions.

Code in this directory should generally:

- be executable from repository test commands such as `make test` and
  `make python-test`;
- verify behavior rather than explain normal usage;
- keep testing helpers separate from the public C library;
- include experimental Python prototypes only when they are clearly outside the
  shipped hpqlib API.

User-facing demonstrations can live in the README while the project stays
small.




## Learning-Augmented Priority Queue Experiments

The paper `docs/papers/learning_augmented_priority_queues.pdf` studies priority
queues in a learning-augmented setting: operations may use predictions that can
be accurate, noisy, or adversarial, but the data structure must still preserve
the priority queue semantics.

The paper focuses on three prediction models.


### Dirty Comparisons

In this model, comparing two keys has two versions:

- a dirty comparison, which is fast or cheap but may be wrong;
- a clean comparison, which gives the true ordering.

The paper uses dirty comparisons to guess where an item should go, then clean
comparisons to repair the guess.  For skip-list priority queues, insertion is
described as a dirty search followed by a clean exponential search.  Tests for
this model should keep separate counters for dirty and clean comparisons and
should verify that bad dirty comparisons do not break the final extracted order.


### Pointer Predictions

In this model, each insertion receives a prediction of the predecessor of the
new key in the current queue.  If the predicted predecessor is close to the true
predecessor, insertion should be cheaper.

Tests for this model should explicitly track predecessor prediction error, i.e.
the rank distance between the predicted predecessor and the true predecessor.
This model is most naturally connected to skip-list based queues, because a
known predecessor gives a useful starting point for local search.

### Rank Predictions

In this model, each inserted key receives a prediction of its final rank among
all keys that will be inserted.  The paper combines this with an auxiliary
predecessor structure and a skip list.

This is the simplest model to prototype in Python tests because test fixtures can
compute true ranks directly.  Useful test cases include:

- perfect rank predictions;
- noisy rank predictions with bounded error;
- adversarial or reversed rank predictions;
- sorting by inserting all items and repeatedly extracting the minimum.

The tests should measure the rank-prediction error
`abs(true_rank - predicted_rank)` and, where relevant, the maximum error over the
test instance.

## Testing Direction

Keep two concerns separate:

- `hpqlib.PriorityQueue` tests verify the C library and its CPython binding;
- learning-augmented tests verify Python-side experimental prototypes that use
  `hpqlib.PriorityQueue` as a building block.

Current files follow this split:

- `python_priority_queue_test.py` checks the CPython binding for the native
  priority queue;
- `pq_experiments/` contains focused modules for graph parsing, Dijkstra
  tracing, prediction statistics, result-output helpers, and prototype queues;
- `learning_augmented_priority_queue_test.py` checks those prototypes.

The experimental code in this directory is not part of the shipped hpqlib API.
It should be treated as a small laboratory for exploring the paper's models
before deciding which ideas deserve C implementations.

The first learning-augmented prototype should probably target rank predictions:
it is easy to generate deterministic fixtures, compute true ranks, and compare
perfect, noisy, and bad predictions while always checking that `ExtractMin`
returns the correct sorted order.

The current Python experiments also include a lazy Dijkstra runner using
`hpqlib.PriorityQueue` as its backend.  Its purpose is not to change the C data
structure, but to collect traces from graph-search runs:

- final distances;
- node extraction order;
- keys inserted into the priority queue;
- per-push prediction metadata;
- aggregate Dijkstra counters such as pushes, pops, stale pops, extractions, and
  reached nodes.

Those traces can be reused to build simple predictors for later runs, such as:

- node-rank predictions from a previous extraction order;
- key-rank predictions from keys inserted in a previous run.

This lets the tests measure whether previous graph searches provide useful
prediction signals before any optional hint API is added to the C library.

DIMACS shortest-path graphs, such as road-network `.gr` files from the 9th
DIMACS Challenge, can be loaded with `load_dimacs_graph(path)`.  The loader uses
a CSR representation backed by standard-library `array` objects.  The automatic
tests use small generated fixtures; large real road networks should be reserved
for manual checks or future benchmark scripts, not mandatory quick tests.

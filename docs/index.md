# pqlib Documentation

pqlib is a C99 priority-queue library with a CPython extension module.

The project exposes one abstract priority-queue API and lets callers choose the
concrete backend at construction time. This keeps client code independent from
the data structure used internally.

## Documentation Map

- [C API](c-api.md): public C interface, ownership rules, and examples.
- [Python API](python-api.md): `pqlib.PriorityQueue` usage and behavior.
- [Build And Packaging](building.md): local builds, tests, installation, and
  wheel generation.

## Implementations

| Implementation | C selector | Python selector | Status |
| --- | --- | --- | --- |
| Binary heap | `PRIORITY_QUEUE_BINARY_HEAP` | `"binary_heap"` | implemented |
| Randomized skiplist | `PRIORITY_QUEUE_RANDOMIZED_SKIPLIST` | `"randomized_skiplist"` | implemented |
| Fibonacci heap | `PRIORITY_QUEUE_FIBONACCI_HEAP` | `"fibonacci_heap"` | planned |
| Kaplan heap | `PRIORITY_QUEUE_KAPLAN_HEAP` | `"kaplan_heap"` | planned |
| Deterministic skiplist | `PRIORITY_QUEUE_DETERMINISTIC_SKIPLIST` | `"deterministic_skiplist"` | planned |
| Chunked skiplist | `PRIORITY_QUEUE_CHUNKED_SKIPLIST` | `"chunked_skiplist"` | planned |

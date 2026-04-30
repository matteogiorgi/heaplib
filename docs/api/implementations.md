# Heap Implementations

hpqlib is intentionally limited to heap-priority-queue implementations. All
backends are selected through `priority_queue_create()` and then used through
the same public `priority_queue` API.

## Summary

| Backend | C selector | Python selector | Status |
| --- | --- | --- | --- |
| Binary heap | `PRIORITY_QUEUE_BINARY_HEAP` | `"binary_heap"` | implemented |
| Fibonacci heap | `PRIORITY_QUEUE_FIBONACCI_HEAP` | `"fibonacci_heap"` | stub |
| Kaplan heap | `PRIORITY_QUEUE_KAPLAN_HEAP` | `"kaplan_heap"` | stub |

## binary_heap

Status: implemented.

Source files:

- `src/heaps/binary_heap.c`
- `src/heaps/binary_heap.h`

Data structure:

- contiguous binary min-heap array;
- comparator-driven ordering;
- caller-owned item pointers.

Expected operation costs:

| Operation | Cost |
| --- | --- |
| `priority_queue_push` | `O(log n)` |
| `priority_queue_peek` | `O(1)` |
| `priority_queue_pop` | `O(log n)` |
| `priority_queue_size` | `O(1)` |
| `priority_queue_empty` | `O(1)` |

Notes:

- equal-priority items are not guaranteed to pop in insertion order;
- allocation grows the backing array as needed.

## fibonacci_heap

Status: stub, not implemented.

Source files:

- `src/heaps/fibonacci_heap.c`
- `src/heaps/fibonacci_heap.h`

Current behavior:

- the C factory recognizes `PRIORITY_QUEUE_FIBONACCI_HEAP`;
- `fibonacci_heap_create()` returns `NULL`;
- the Python selector `"fibonacci_heap"` raises `NotImplementedError`.

## kaplan_heap

Status: stub, not implemented.

Source files:

- `src/heaps/kaplan_heap.c`
- `src/heaps/kaplan_heap.h`

This backend is reserved for the Fibonacci-heaps-revisited structure referred
to in this project as a Kaplan heap.

Current behavior:

- the C factory recognizes `PRIORITY_QUEUE_KAPLAN_HEAP`;
- `kaplan_heap_create()` returns `NULL`;
- the Python selector `"kaplan_heap"` raises `NotImplementedError`.

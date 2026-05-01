# Heap Implementations

hpqlib is intentionally limited to heap-priority-queue implementations. All
backends are selected through `priority_queue_create()` and then used through
the same public `priority_queue` API.

The targeted operations `priority_queue_decrease_key` and
`priority_queue_remove` use handles returned by `priority_queue_push_handle`.
This matches the usual heap assumption that the item's position is known.
`priority_queue_contains` remains a pointer-identity convenience query and is
linear in the current backends.

## Summary

| Backend | C selector | Python selector | Status |
| --- | --- | --- | --- |
| Binary heap | `PRIORITY_QUEUE_BINARY_HEAP` | `"binary_heap"` | implemented |
| Fibonacci heap | `PRIORITY_QUEUE_FIBONACCI_HEAP` | `"fibonacci_heap"` | implemented |
| Kaplan heap | `PRIORITY_QUEUE_KAPLAN_HEAP` | `"kaplan_heap"` | implemented |

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
| `priority_queue_push_handle` | `O(log n)` |
| `priority_queue_decrease_key` | `O(log n)` |
| `priority_queue_remove` | `O(log n)` |
| `priority_queue_contains` | `O(n)` |
| `priority_queue_peek` | `O(1)` |
| `priority_queue_pop` | `O(log n)` |
| `priority_queue_size` | `O(1)` |
| `priority_queue_empty` | `O(1)` |

Notes:

- equal-priority items are not guaranteed to pop in insertion order;
- allocation grows the backing array as needed.

## fibonacci_heap

Status: implemented.

Source files:

- `src/heaps/fibonacci_heap.c`
- `src/heaps/fibonacci_heap.h`

Data structure:

- circular doubly linked root list;
- circular doubly linked child lists;
- minimum-node pointer;
- delete-min consolidation by root degree.

Expected operation costs:

| Operation | Cost |
| --- | --- |
| `priority_queue_push` | amortized `O(1)` |
| `priority_queue_push_handle` | amortized `O(1)` |
| `priority_queue_decrease_key` | amortized `O(1)` |
| `priority_queue_remove` | amortized `O(log n)` |
| `priority_queue_contains` | `O(n)` |
| `priority_queue_peek` | `O(1)` |
| `priority_queue_pop` | amortized `O(log n)` |
| `priority_queue_size` | `O(1)` |
| `priority_queue_empty` | `O(1)` |

Notes:

- the node layout keeps parent/child/degree/mark fields for consolidation and
  cascading cuts;
- `priority_queue_remove` follows the paper's arbitrary-delete operation:
  handled non-minimum nodes are cut or removed from the root list, their
  children are promoted, and cascading cuts repair ancestors;
- equal-priority items are not guaranteed to pop in insertion order.

## kaplan_heap

Status: implemented.

Source files:

- `src/heaps/kaplan_heap.c`
- `src/heaps/kaplan_heap.h`

This backend implements the simple Fibonacci heap from "Fibonacci Heaps
Revisited", referred to in this project as a Kaplan heap.

Data structure:

- one heap-ordered tree per queue;
- doubly linked child lists;
- rank per node;
- delete-min consolidation through fair links, followed by naive links.

Expected operation costs:

| Operation | Cost |
| --- | --- |
| `priority_queue_push` | amortized `O(1)` |
| `priority_queue_push_handle` | amortized `O(1)` |
| `priority_queue_decrease_key` | amortized `O(1)` cascading rank repair |
| `priority_queue_remove` | amortized `O(log n)` via delete-by-handle |
| `priority_queue_contains` | `O(n)` |
| `priority_queue_peek` | `O(1)` |
| `priority_queue_pop` | amortized `O(log n)` |
| `priority_queue_size` | `O(1)` |
| `priority_queue_empty` | `O(1)` |

Notes:

- the node layout keeps parent/child/rank/mark fields for fair links and
  cascading rank repairs;
- `priority_queue_remove` simulates the paper's delete operation internally by
  cutting the handled node, making it the logical minimum, and reusing
  delete-min;
- equal-priority items are not guaranteed to pop in insertion order.

# pqlib

pqlib is a C99 library for experimenting with generic priority queues.

The project aims to expose a common abstract priority-queue API while allowing
multiple concrete implementations to coexist behind the same logical interface.
The planned implementations include binary heaps, Fibonacci heaps, Kaplan heaps
(revisited Fibonacci heaps), randomized skiplists, deterministic skiplists,
chunked skiplists, and other data structures useful for performance
comparisons.

The design follows an object-oriented style adapted idiomatically to C:
interfaces and implementations are kept separate, and concrete priority queues
can be selected at creation time while client code uses the shared API.




## Architecture

pqlib has one public abstract data type:

- `priority_queue`

The implementation families are only organizational categories:

- `heaps/`
- `skiplists/`

Concrete implementations provide the `priority_queue` behavior directly:

- `binary_heap`
- `fibonacci_heap`
- `kaplan_heap`
- `randomized_skiplist`
- `deterministic_skiplist`
- `chunked_skiplist`

There is a single public abstract API, `priority_queue`. The `heaps/` and
`skiplists/` families are not intermediate runtime abstractions, and concrete
implementations are expected to provide the `priority_queue` vtable directly.
Intermediate vtables for heaps or skiplists may be introduced later only if a
clear need emerges.




## Priority Queue Operations

The initial priority-queue interface is expected to provide:

- `create`
- `destroy`
- `push`
- `peek`
- `pop`
- `size`
- `empty`

More advanced operations may be added later:

- `decrease_key`
- `remove`
- `contains`




## Repository Layout

- `include/`: public headers installed or consumed by client code.
- `include/pqlib/`: public namespace for pqlib headers.
- `include/pqlib/priority_queue.h`: abstract priority-queue API, comparator
  type, implementation selector, and public operations.
- `src/`: private implementation sources for the library.
- `src/priority_queue.c`: public API dispatch, null handling, and factory that
  selects a concrete implementation.
- `src/priority_queue_internal.h`: private base object layout and vtable used by
  concrete implementations.
- `src/heaps/`: concrete heap-based priority-queue implementations.
- `src/heaps/binary_heap.h`: private factory for the binary-heap backend.
- `src/heaps/binary_heap.c`: binary min-heap backend implementing the
  `priority_queue` vtable.
- `examples/`: small programs showing how to use the public API.
- `examples/priority_queue_demo.c`: demo that creates a binary-heap-backed
  priority queue and prints integers in priority order.
- `tests/`: automated tests for public behavior.
- `tests/priority_queue_test.c`: assert-based tests for create, empty, push,
  peek, pop, size, duplicate handling, and invalid comparator handling.
- `legacy/`: reference code kept during the transition to the abstract API.
- `legacy/binaryheap/`: original direct binary-heap implementation.
- `legacy/binaryheap/binaryheap.h`: old public header for the standalone binary
  heap.
- `legacy/binaryheap/binaryheap.c`: old generic min-heap implementation based on
  `void **`.
- `legacy/binaryheap/binaryheap_demo.c`: old demo using the standalone
  `binaryheap` API.
- `legacy/binaryheap/binaryheap_test.c`: old assert-based tests for the
  standalone `binaryheap` API.
- `ISTRUZIONI.txt`: project notes and architectural requirements.
- `Makefile`: builds the static library, demo, and tests.
- `.gitignore`: excludes build outputs and local development artifacts.

The legacy `binaryheap.*` files are kept as a direct implementation reference,
but the active library entry point is now the abstract `priority_queue` API.




## Useful Commands

- `make`: builds `build/libpqlib.a`
- `make run`: builds and runs the demo
- `make test`: builds and runs the tests
- `make clean`: removes build artifacts

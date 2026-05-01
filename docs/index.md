# hpqlib Documentation

hpqlib is a C99 heap-priority-queue library with a small CPython binding.

The C API reference is generated from the public and internal C sources with
Doxygen. The Markdown files in this directory provide architecture notes,
backend summaries, Python binding notes, and build instructions.

## Reference

- C API reference: run `make docs` and open `build/docs/html/index.html`.
- [Heap Implementations](implementations.md): available heap backends.
- [Python API](python.md): `hpqlib.PriorityQueue` constructor, methods, and
  exceptions.

## Build And Packaging

- [Build And Packaging](building.md): local C builds, Python extension builds,
  wheels, and release workflow.

## Papers

Reference papers used while developing the project live under `docs/papers/`.

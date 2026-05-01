# Graph Datasets

This directory is reserved for local graph datasets used in manual checks and
future benchmarks. Dataset files are intentionally not tracked by git.

The current Python loader supports DIMACS shortest-path `.gr` files with lines
like:

```text
c optional comments
p sp <node-count> <arc-count>
a <tail> <head> <weight>
```

Road-network datasets in this format were published for the 9th DIMACS
Implementation Challenge. Put downloaded DIMACS `.gr` files under
`graphs/dimacs/` when you want to run local experiments.

The C Dijkstra benchmark can be run against one of these files with:

```sh
make benchmark GRAPH=graphs/dimacs/USA-road-d.NY.gr SOURCE=1
```

The Python Dijkstra benchmark uses the same files:

```sh
make python-benchmark GRAPH=graphs/dimacs/USA-road-d.NY.gr SOURCE=1
```

The default smoke benchmark uses `graphs/dimacs/USA-road-d.NY.gr`. It loads the
graph into CSR memory, runs Dijkstra with the binary, Fibonacci, and Kaplan heap
backends, checks that the three distance checksums match, and prints the elapsed
time for each run.

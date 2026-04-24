# Python API

The Python binding exposes a CPython extension module named `pqlib`.

```python
import pqlib
```

## PriorityQueue

```python
queue = pqlib.PriorityQueue(implementation="binary_heap")
```

The `implementation` argument is optional and defaults to `"binary_heap"`.

Available selectors:

- `"binary_heap"`
- `"randomized_skiplist"`

Planned selectors:

- `"fibonacci_heap"`
- `"kaplan_heap"`
- `"deterministic_skiplist"`
- `"chunked_skiplist"`

## Ordering

Items are ordered using Python's natural ordering. Objects inserted into the
same queue must therefore be mutually comparable with `<` and `>`.

With normal Python values such as integers, lower values are popped first.

## Methods

```python
queue.push(item)
queue.peek()
queue.pop()
queue.size()
queue.empty()
```

`peek()` returns the highest-priority item without removing it.

`pop()` removes and returns the highest-priority item.

`peek()` and `pop()` return `None` when the queue is empty.

The queue also supports:

```python
len(queue)
bool(queue)
```

## Reference Ownership

The binding stores strong references to Python objects while they are inside the
queue. Popping an object returns that reference to Python. Destroying a queue
releases any objects that were still stored in it.

## Example

```python
import pqlib

queue = pqlib.PriorityQueue(implementation="randomized_skiplist")

queue.push(5)
queue.push(1)
queue.push(3)

assert queue.peek() == 1
assert queue.pop() == 1
assert len(queue) == 2
assert bool(queue)
```

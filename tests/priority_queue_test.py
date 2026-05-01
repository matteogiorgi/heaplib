import pathlib
import sys
from dataclasses import dataclass, field

# Allow direct execution from the source tree after an in-place extension build.
sys.path.insert(0, str(pathlib.Path(__file__).resolve().parents[1]))

import hpqlib


IMPLEMENTATIONS = (
    "binary_heap",
    "fibonacci_heap",
    "kaplan_heap",
)


@dataclass(order=True)
class KeyedValue:
    priority: int
    sequence: int
    name: str = field(compare=False)


def drain(queue):
    values = []
    while queue:
        values.append(queue.pop())
    return values


def check_empty_queue(implementation):
    queue = hpqlib.PriorityQueue(implementation=implementation)

    assert not queue
    assert len(queue) == 0
    assert queue.size() == 0
    assert queue.empty()
    assert queue.peek() is None
    assert queue.pop() is None


def check_push_pop_order(implementation):
    queue = hpqlib.PriorityQueue(implementation=implementation)

    for value in [7, 3, 9, 1, 4, 8, 2]:
        queue.push(value)

    assert queue
    assert len(queue) == 7
    assert queue.size() == 7
    assert queue.peek() == 1
    assert drain(queue) == [1, 2, 3, 4, 7, 8, 9]
    assert queue.pop() is None
    assert queue.empty()


def check_duplicates(implementation):
    queue = hpqlib.PriorityQueue(implementation=implementation)

    for value in [5, 1, 5, 1, 3]:
        queue.push(value)

    assert drain(queue) == [1, 1, 3, 5, 5]


def check_interleaved_operations(implementation):
    queue = hpqlib.PriorityQueue(implementation=implementation)

    for value in [10, 4, 7]:
        queue.push(value)
    assert queue.pop() == 4

    for value in [1, 6]:
        queue.push(value)
    assert queue.pop() == 1

    queue.push(3)
    assert drain(queue) == [3, 6, 7, 10]


def check_larger_deterministic_order(implementation):
    queue = hpqlib.PriorityQueue(implementation=implementation)
    values = [(index * 37) % 101 for index in range(256)]

    for value in values:
        queue.push(value)

    assert drain(queue) == sorted(values)


def check_object_ordering(implementation):
    queue = hpqlib.PriorityQueue(implementation=implementation)
    items = [
        KeyedValue(30, 0, "slow"),
        KeyedValue(10, 1, "fast"),
        KeyedValue(20, 2, "middle"),
    ]

    for item in items:
        queue.push(item)

    assert queue.peek().name == "fast"
    assert [item.name for item in drain(queue)] == ["fast", "middle", "slow"]


def check_contains_and_remove(implementation):
    queue = hpqlib.PriorityQueue(implementation=implementation)
    values = [
        KeyedValue(7, 0, "seven"),
        KeyedValue(3, 1, "three"),
        KeyedValue(9, 2, "nine"),
        KeyedValue(1, 3, "one"),
        KeyedValue(4, 4, "four"),
    ]
    missing = KeyedValue(42, 5, "missing")
    handles = [queue.push_handle(value) for value in values]

    assert isinstance(handles[2], hpqlib.PriorityQueueHandle)
    assert queue.contains(values[2])
    assert not queue.contains(missing)
    assert queue.remove(handles[2]) is values[2]
    assert not queue.contains(values[2])
    assert len(queue) == 4

    try:
        queue.remove(handles[2])
    except ValueError:
        pass
    else:
        raise AssertionError("stale handle was accepted after remove")

    assert [item.name for item in drain(queue)] == [
        "one",
        "three",
        "four",
        "seven",
    ]


def check_decrease_key(implementation):
    queue = hpqlib.PriorityQueue(implementation=implementation)
    slow = KeyedValue(30, 0, "slow")
    fast = KeyedValue(10, 1, "fast")
    middle = KeyedValue(20, 2, "middle")

    slow_handle = queue.push_handle(slow)
    queue.push(fast)
    queue.push(middle)
    assert queue.peek() is fast

    slow.priority = 5
    queue.decrease_key(slow_handle)
    assert queue.peek() is slow

    assert queue.pop() is slow
    try:
        queue.decrease_key(slow_handle)
    except ValueError:
        pass
    else:
        raise AssertionError("stale handle was accepted after pop")

    assert [item.name for item in drain(queue)] == ["fast", "middle"]


def check_handle_belongs_to_queue(implementation):
    left = hpqlib.PriorityQueue(implementation=implementation)
    right = hpqlib.PriorityQueue(implementation=implementation)
    handle = left.push_handle(KeyedValue(1, 0, "left"))

    try:
        right.remove(handle)
    except ValueError:
        pass
    else:
        raise AssertionError("handle was accepted by the wrong queue")


def check_handle_invalid_after_reinitialization(implementation):
    queue = hpqlib.PriorityQueue(implementation=implementation)
    handle = queue.push_handle(KeyedValue(1, 0, "old"))

    queue.__init__(implementation=implementation)

    try:
        queue.decrease_key(handle)
    except ValueError:
        pass
    else:
        raise AssertionError("handle survived queue reinitialization")


def test_invalid_implementation():
    try:
        hpqlib.PriorityQueue(implementation="missing")
    except ValueError:
        return

    raise AssertionError("invalid implementation was accepted")


def main():
    test_invalid_implementation()

    for implementation in IMPLEMENTATIONS:
        check_empty_queue(implementation)
        check_push_pop_order(implementation)
        check_duplicates(implementation)
        check_interleaved_operations(implementation)
        check_larger_deterministic_order(implementation)
        check_object_ordering(implementation)
        check_contains_and_remove(implementation)
        check_decrease_key(implementation)
        check_handle_belongs_to_queue(implementation)
        check_handle_invalid_after_reinitialization(implementation)

    print("All Python priority_queue tests passed")


if __name__ == "__main__":
    main()

# C API

The public C API is declared in:

```c
#include "pqlib/priority_queue.h"
```

## Queue Type

`struct priority_queue` is opaque. Client code must create queues with
`priority_queue_create()` and release them with `priority_queue_destroy()`.

```c
struct priority_queue;
```

## Comparator

```c
typedef int (*priority_queue_cmp_fn)(const void *lhs, const void *rhs);
```

The comparator defines priority order:

- return a negative value if `lhs` has higher priority than `rhs`;
- return zero if both items have equivalent priority;
- return a positive value if `lhs` has lower priority than `rhs`.

With a normal ascending comparator, smaller values are popped first.

## Ownership

The C queue stores `void *` item pointers. It does not copy, free, or otherwise
own the objects those pointers refer to.

The caller is responsible for ensuring that stored objects remain valid while
they are inside the queue. Destroying a queue releases only memory allocated by
the queue implementation itself.

## Operations

```c
struct priority_queue *priority_queue_create(
    enum priority_queue_implementation implementation,
    priority_queue_cmp_fn cmp
);
void priority_queue_destroy(struct priority_queue *queue);
int priority_queue_push(struct priority_queue *queue, void *item);
void *priority_queue_peek(const struct priority_queue *queue);
void *priority_queue_pop(struct priority_queue *queue);
size_t priority_queue_size(const struct priority_queue *queue);
int priority_queue_empty(const struct priority_queue *queue);
```

`priority_queue_create()` returns `NULL` if the implementation is unsupported,
the comparator is `NULL`, or allocation fails.

`priority_queue_push()` returns `0` on success and `-1` on failure.

`priority_queue_peek()` and `priority_queue_pop()` return `NULL` when the queue
is empty.

## Example

```c
#include <stdio.h>

#include "pqlib/priority_queue.h"

static int int_cmp(const void *lhs, const void *rhs)
{
    const int *left = lhs;
    const int *right = rhs;

    if (*left < *right)
        return -1;
    if (*left > *right)
        return 1;
    return 0;
}

int main(void)
{
    int values[] = { 7, 3, 9, 1 };
    struct priority_queue *queue;
    size_t i;

    queue = priority_queue_create(PRIORITY_QUEUE_BINARY_HEAP, int_cmp);
    if (queue == NULL)
        return 1;

    for (i = 0; i < sizeof(values) / sizeof(values[0]); i++)
        priority_queue_push(queue, &values[i]);

    while (!priority_queue_empty(queue)) {
        int *value = priority_queue_pop(queue);
        printf("%d\n", *value);
    }

    priority_queue_destroy(queue);
    return 0;
}
```

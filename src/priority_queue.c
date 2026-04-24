#include <stddef.h>

#include "pqlib/priority_queue.h"
#include "priority_queue_internal.h"
#include "heaps/binary_heap.h"
#include "skiplists/randomized_skiplist.h"

/*
 * Initialize the base object shared by all concrete priority queues.
 */
void priority_queue_init(
    struct priority_queue *queue,
    const struct priority_queue_vtable *vtable,
    priority_queue_cmp_fn cmp
)
{
    queue->vtable = vtable;
    queue->cmp = cmp;
}

/*
 * Public factory for priority queues.
 *
 * The factory keeps implementation selection centralized so callers do not
 * depend on private backend headers or concrete object layouts.
 */
struct priority_queue *priority_queue_create(
    enum priority_queue_implementation implementation,
    priority_queue_cmp_fn cmp
)
{
    if (cmp == NULL)
        return NULL;

    switch (implementation) {
    case PRIORITY_QUEUE_BINARY_HEAP:
        return binary_heap_create(cmp);
    case PRIORITY_QUEUE_RANDOMIZED_SKIPLIST:
        return randomized_skiplist_create(cmp);
    default:
        return NULL;
    }
}

/*
 * Public API dispatch functions.
 *
 * These wrappers define the NULL-handle behavior once and keep concrete
 * implementations focused on valid queue instances.
 */
void priority_queue_destroy(struct priority_queue *queue)
{
    if (queue == NULL)
        return;

    queue->vtable->destroy(queue);
}

int priority_queue_push(struct priority_queue *queue, void *item)
{
    if (queue == NULL)
        return -1;

    return queue->vtable->push(queue, item);
}

void *priority_queue_peek(const struct priority_queue *queue)
{
    if (queue == NULL)
        return NULL;

    return queue->vtable->peek(queue);
}

void *priority_queue_pop(struct priority_queue *queue)
{
    if (queue == NULL)
        return NULL;

    return queue->vtable->pop(queue);
}

size_t priority_queue_size(const struct priority_queue *queue)
{
    if (queue == NULL)
        return 0;

    return queue->vtable->size(queue);
}

int priority_queue_empty(const struct priority_queue *queue)
{
    if (queue == NULL)
        return 1;

    return queue->vtable->empty(queue);
}

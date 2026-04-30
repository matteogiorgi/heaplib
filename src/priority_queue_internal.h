#ifndef HPQLIB_PRIORITY_QUEUE_INTERNAL_H
#define HPQLIB_PRIORITY_QUEUE_INTERNAL_H

#include <stddef.h>

#include "hpqlib/priority_queue.h"

/*
 * Implementation dispatch table for the abstract priority_queue API.
 *
 * Each concrete backend provides one static vtable and implements all entries
 * using the common base pointer. Public functions perform basic NULL handling
 * before dispatching through this table.
 */
struct priority_queue_vtable {
    void (*destroy)(struct priority_queue *queue);
    int (*push)(struct priority_queue *queue, void *item);
    void *(*peek)(const struct priority_queue *queue);
    void *(*pop)(struct priority_queue *queue);
    size_t(*size)(const struct priority_queue *queue);
    int (*empty)(const struct priority_queue *queue);
};

/*
 * Common header embedded as the first field of every concrete priority queue.
 *
 * This layout allows a concrete implementation pointer to be safely exposed as
 * struct priority_queue * and later recovered by the implementation-specific
 * cast helpers.
 */
struct priority_queue {
    const struct priority_queue_vtable *vtable;
    priority_queue_cmp_fn cmp;
};

/*
 * Initialize the common priority_queue base object.
 *
 * Concrete constructors call this after allocating their full implementation
 * object and before returning the abstract handle to client code.
 */
void priority_queue_init(
    struct priority_queue *queue,
    const struct priority_queue_vtable *vtable,
    priority_queue_cmp_fn cmp
);

#endif

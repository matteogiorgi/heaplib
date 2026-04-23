#ifndef PQLIB_PRIORITY_QUEUE_INTERNAL_H
#define PQLIB_PRIORITY_QUEUE_INTERNAL_H

#include <stddef.h>

#include "pqlib/priority_queue.h"

struct priority_queue_vtable {
    void (*destroy)(struct priority_queue *queue);
    int (*push)(struct priority_queue *queue, void *item);
    void *(*peek)(const struct priority_queue *queue);
    void *(*pop)(struct priority_queue *queue);
    size_t (*size)(const struct priority_queue *queue);
    int (*empty)(const struct priority_queue *queue);
};

struct priority_queue {
    const struct priority_queue_vtable *vtable;
    priority_queue_cmp_fn cmp;
};

void priority_queue_init(
    struct priority_queue *queue,
    const struct priority_queue_vtable *vtable,
    priority_queue_cmp_fn cmp
);

#endif

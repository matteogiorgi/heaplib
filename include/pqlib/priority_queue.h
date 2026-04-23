#ifndef PQLIB_PRIORITY_QUEUE_H
#define PQLIB_PRIORITY_QUEUE_H

#include <stddef.h>

typedef int (*priority_queue_cmp_fn)(const void *lhs, const void *rhs);

enum priority_queue_implementation {
    PRIORITY_QUEUE_BINARY_HEAP = 1
};

struct priority_queue;

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

#endif

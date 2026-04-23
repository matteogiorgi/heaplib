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
    struct priority_queue *queue;
    int values[] = { 7, 3, 9, 1, 4, 8, 2 };
    size_t i;

    queue = priority_queue_create(PRIORITY_QUEUE_BINARY_HEAP, int_cmp);
    if (queue == NULL) {
        fprintf(stderr, "priority_queue_create failed\n");
        return 1;
    }

    for (i = 0; i < sizeof(values) / sizeof(values[0]); i++) {
        if (priority_queue_push(queue, &values[i]) != 0) {
            fprintf(stderr, "priority_queue_push failed\n");
            priority_queue_destroy(queue);
            return 1;
        }
    }

    while (!priority_queue_empty(queue)) {
        int *value = priority_queue_pop(queue);
        printf("%d\n", *value);
    }

    priority_queue_destroy(queue);
    return 0;
}

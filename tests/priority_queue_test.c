#include <assert.h>
#include <stddef.h>
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

static void test_create_rejects_missing_cmp(void)
{
    assert(priority_queue_create(PRIORITY_QUEUE_BINARY_HEAP, NULL) == NULL);
    assert(priority_queue_create(PRIORITY_QUEUE_RANDOMIZED_SKIPLIST, NULL) == NULL);
}

static void test_empty_queue(enum priority_queue_implementation implementation)
{
    struct priority_queue *queue;

    queue = priority_queue_create(implementation, int_cmp);
    assert(queue != NULL);

    assert(priority_queue_empty(queue));
    assert(priority_queue_size(queue) == 0);
    assert(priority_queue_peek(queue) == NULL);
    assert(priority_queue_pop(queue) == NULL);

    priority_queue_destroy(queue);
}

static void test_push_pop_order(enum priority_queue_implementation implementation)
{
    struct priority_queue *queue;
    int values[] = { 7, 3, 9, 1, 4, 8, 2 };
    int expected[] = { 1, 2, 3, 4, 7, 8, 9 };
    size_t i;

    queue = priority_queue_create(implementation, int_cmp);
    assert(queue != NULL);

    for (i = 0; i < sizeof(values) / sizeof(values[0]); i++)
        assert(priority_queue_push(queue, &values[i]) == 0);

    assert(priority_queue_size(queue) == sizeof(values) / sizeof(values[0]));
    assert(*(int *) priority_queue_peek(queue) == 1);

    for (i = 0; i < sizeof(expected) / sizeof(expected[0]); i++)
        assert(*(int *) priority_queue_pop(queue) == expected[i]);

    assert(priority_queue_empty(queue));
    priority_queue_destroy(queue);
}

static void test_duplicates(enum priority_queue_implementation implementation)
{
    struct priority_queue *queue;
    int values[] = { 5, 1, 5, 1, 3 };
    int expected[] = { 1, 1, 3, 5, 5 };
    size_t i;

    queue = priority_queue_create(implementation, int_cmp);
    assert(queue != NULL);

    for (i = 0; i < sizeof(values) / sizeof(values[0]); i++)
        assert(priority_queue_push(queue, &values[i]) == 0);

    for (i = 0; i < sizeof(expected) / sizeof(expected[0]); i++)
        assert(*(int *) priority_queue_pop(queue) == expected[i]);

    priority_queue_destroy(queue);
}

int main(void)
{
    enum priority_queue_implementation implementations[] = {
        PRIORITY_QUEUE_BINARY_HEAP,
        PRIORITY_QUEUE_RANDOMIZED_SKIPLIST
    };
    size_t i;

    test_create_rejects_missing_cmp();

    for (i = 0; i < sizeof(implementations) / sizeof(implementations[0]); i++) {
        test_empty_queue(implementations[i]);
        test_push_pop_order(implementations[i]);
        test_duplicates(implementations[i]);
    }

    printf("All priority_queue tests passed\n");
    return 0;
}

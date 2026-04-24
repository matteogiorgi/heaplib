#include <stdlib.h>

#include "priority_queue_internal.h"
#include "skiplists/randomized_skiplist.h"

#define RANDOMIZED_SKIPLIST_MAX_LEVEL 32

/*
 * Randomized skiplist implementation of the abstract priority_queue API.
 *
 * The skiplist stores caller-owned item pointers in ascending priority order.
 * The first node at level 0 is therefore the highest-priority item.
 */
struct randomized_skiplist_node {
    void *item;
    size_t level_count;
    struct randomized_skiplist_node *forward[RANDOMIZED_SKIPLIST_MAX_LEVEL];
};

struct randomized_skiplist {
    struct priority_queue base;
    struct randomized_skiplist_node *head;
    size_t level_count;
    size_t size;
};

static void randomized_skiplist_destroy(struct priority_queue *queue);
static int randomized_skiplist_push(struct priority_queue *queue, void *item);
static void *randomized_skiplist_peek(const struct priority_queue *queue);
static void *randomized_skiplist_pop(struct priority_queue *queue);
static size_t randomized_skiplist_size(const struct priority_queue *queue);
static int randomized_skiplist_empty(const struct priority_queue *queue);

/*
 * Static vtable exposed through the common priority_queue base.
 */
static const struct priority_queue_vtable randomized_skiplist_vtable = {
    randomized_skiplist_destroy,
    randomized_skiplist_push,
    randomized_skiplist_peek,
    randomized_skiplist_pop,
    randomized_skiplist_size,
    randomized_skiplist_empty
};

/*
 * Recover the concrete skiplist object from the abstract base pointer.
 *
 * This cast is valid because struct priority_queue is the first field of
 * struct randomized_skiplist.
 */
static struct randomized_skiplist *randomized_skiplist_from_queue(
    struct priority_queue *queue
)
{
    return (struct randomized_skiplist *)queue;
}

/*
 * Const-preserving variant of randomized_skiplist_from_queue().
 */
static const struct randomized_skiplist *randomized_skiplist_from_const_queue(
    const struct priority_queue *queue
)
{
    return (const struct randomized_skiplist *)queue;
}

/*
 * Allocate a skiplist node with all forward links initialized to NULL.
 */
static struct randomized_skiplist_node *randomized_skiplist_node_create(
    void *item,
    size_t level_count
)
{
    struct randomized_skiplist_node *node;
    size_t i;

    node = malloc(sizeof(*node));
    if (node == NULL)
        return NULL;

    node->item = item;
    node->level_count = level_count;
    for (i = 0; i < RANDOMIZED_SKIPLIST_MAX_LEVEL; i++)
        node->forward[i] = NULL;

    return node;
}

/*
 * Select a random node height using repeated fair coin flips.
 */
static size_t randomized_skiplist_random_level(void)
{
    size_t level_count = 1;

    while (
        level_count < RANDOMIZED_SKIPLIST_MAX_LEVEL &&
        (rand() & 1) != 0
        )
        level_count++;

    return level_count;
}

/*
 * Allocate an empty randomized-skiplist-backed priority queue.
 */
struct priority_queue *randomized_skiplist_create(priority_queue_cmp_fn cmp)
{
    struct randomized_skiplist *skiplist;

    skiplist = malloc(sizeof(*skiplist));
    if (skiplist == NULL)
        return NULL;

    priority_queue_init(&skiplist->base, &randomized_skiplist_vtable, cmp);
    skiplist->head = randomized_skiplist_node_create(NULL, RANDOMIZED_SKIPLIST_MAX_LEVEL);
    if (skiplist->head == NULL) {
        free(skiplist);
        return NULL;
    }

    skiplist->level_count = 1;
    skiplist->size = 0;

    return &skiplist->base;
}

/*
 * Release all skiplist-owned nodes.
 *
 * Stored item pointers are caller-owned and are intentionally left untouched.
 */
static void randomized_skiplist_destroy(struct priority_queue *queue)
{
    struct randomized_skiplist *skiplist =
        randomized_skiplist_from_queue(queue);
    struct randomized_skiplist_node *node = skiplist->head;

    while (node != NULL) {
        struct randomized_skiplist_node *next = node->forward[0];

        free(node);
        node = next;
    }

    free(skiplist);
}

/*
 * Insert an item in ascending priority order.
 */
static int randomized_skiplist_push(struct priority_queue *queue, void *item)
{
    struct randomized_skiplist *skiplist =
        randomized_skiplist_from_queue(queue);
    struct randomized_skiplist_node *update[RANDOMIZED_SKIPLIST_MAX_LEVEL];
    struct randomized_skiplist_node *node;
    size_t new_level_count;
    size_t i;

    node = skiplist->head;
    for (i = skiplist->level_count; i > 0; i--) {
        size_t level = i - 1;

        while (
            node->forward[level] != NULL &&
            skiplist->base.cmp(node->forward[level]->item, item) <= 0
            )
            node = node->forward[level];

        update[level] = node;
    }

    new_level_count = randomized_skiplist_random_level();
    if (new_level_count > skiplist->level_count) {
        for (i = skiplist->level_count; i < new_level_count; i++)
            update[i] = skiplist->head;
        skiplist->level_count = new_level_count;
    }

    node = randomized_skiplist_node_create(item, new_level_count);
    if (node == NULL)
        return -1;

    for (i = 0; i < new_level_count; i++) {
        node->forward[i] = update[i]->forward[i];
        update[i]->forward[i] = node;
    }

    skiplist->size++;
    return 0;
}

/*
 * Return the first level-0 item without modifying the skiplist.
 */
static void *randomized_skiplist_peek(const struct priority_queue *queue)
{
    const struct randomized_skiplist *skiplist =
        randomized_skiplist_from_const_queue(queue);
    const struct randomized_skiplist_node *node = skiplist->head->forward[0];

    if (node == NULL)
        return NULL;

    return node->item;
}

/*
 * Remove and return the first level-0 item.
 */
static void *randomized_skiplist_pop(struct priority_queue *queue)
{
    struct randomized_skiplist *skiplist =
        randomized_skiplist_from_queue(queue);
    struct randomized_skiplist_node *node = skiplist->head->forward[0];
    void *item;
    size_t i;

    if (node == NULL)
        return NULL;

    for (i = 0; i < skiplist->level_count; i++) {
        if (skiplist->head->forward[i] != node)
            break;
        skiplist->head->forward[i] = node->forward[i];
    }

    while (
        skiplist->level_count > 1 &&
        skiplist->head->forward[skiplist->level_count - 1] == NULL
        )
        skiplist->level_count--;

    item = node->item;
    free(node);
    skiplist->size--;

    return item;
}

/*
 * Return the current number of stored items.
 */
static size_t randomized_skiplist_size(const struct priority_queue *queue)
{
    return randomized_skiplist_from_const_queue(queue)->size;
}

/*
 * Return whether the skiplist contains no items.
 */
static int randomized_skiplist_empty(const struct priority_queue *queue)
{
    return randomized_skiplist_size(queue) == 0;
}

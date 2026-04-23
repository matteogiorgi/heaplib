#include <stdlib.h>

#include "heaps/binary_heap.h"
#include "priority_queue_internal.h"

struct binary_heap {
    struct priority_queue base;
    void **data;
    size_t size;
    size_t capacity;
};

static void binary_heap_destroy(struct priority_queue *queue);
static int binary_heap_push(struct priority_queue *queue, void *item);
static void *binary_heap_peek(const struct priority_queue *queue);
static void *binary_heap_pop(struct priority_queue *queue);
static size_t binary_heap_size(const struct priority_queue *queue);
static int binary_heap_empty(const struct priority_queue *queue);

static const struct priority_queue_vtable binary_heap_vtable = {
    binary_heap_destroy,
    binary_heap_push,
    binary_heap_peek,
    binary_heap_pop,
    binary_heap_size,
    binary_heap_empty
};

static struct binary_heap *binary_heap_from_queue(struct priority_queue *queue)
{
    return (struct binary_heap *) queue;
}

static const struct binary_heap *binary_heap_from_const_queue(
    const struct priority_queue *queue
)
{
    return (const struct binary_heap *) queue;
}

static void binary_heap_swap(void **lhs, void **rhs)
{
    void *tmp = *lhs;
    *lhs = *rhs;
    *rhs = tmp;
}

static void binary_heap_sift_up(struct binary_heap *heap, size_t index)
{
    while (index > 0) {
        size_t parent = (index - 1) / 2;

        if (heap->base.cmp(heap->data[index], heap->data[parent]) >= 0)
            break;

        binary_heap_swap(&heap->data[index], &heap->data[parent]);
        index = parent;
    }
}

static void binary_heap_sift_down(struct binary_heap *heap, size_t index)
{
    size_t left, right, smallest;

    for (;;) {
        left = 2 * index + 1;
        right = left + 1;
        smallest = index;

        if (
            left < heap->size &&
            heap->base.cmp(heap->data[left], heap->data[smallest]) < 0
        )
            smallest = left;

        if (
            right < heap->size &&
            heap->base.cmp(heap->data[right], heap->data[smallest]) < 0
        )
            smallest = right;

        if (smallest == index)
            break;

        binary_heap_swap(&heap->data[index], &heap->data[smallest]);
        index = smallest;
    }
}

static int binary_heap_reserve(struct binary_heap *heap, size_t capacity)
{
    void **new_data;

    if (capacity <= heap->capacity)
        return 0;

    new_data = realloc(heap->data, capacity * sizeof(*heap->data));
    if (new_data == NULL)
        return -1;

    heap->data = new_data;
    heap->capacity = capacity;
    return 0;
}

struct priority_queue *binary_heap_create(priority_queue_cmp_fn cmp)
{
    struct binary_heap *heap;

    heap = malloc(sizeof(*heap));
    if (heap == NULL)
        return NULL;

    priority_queue_init(&heap->base, &binary_heap_vtable, cmp);
    heap->data = NULL;
    heap->size = 0;
    heap->capacity = 0;

    return &heap->base;
}

static void binary_heap_destroy(struct priority_queue *queue)
{
    struct binary_heap *heap = binary_heap_from_queue(queue);

    free(heap->data);
    free(heap);
}

static int binary_heap_push(struct priority_queue *queue, void *item)
{
    struct binary_heap *heap = binary_heap_from_queue(queue);
    size_t new_capacity;

    if (heap->size == heap->capacity) {
        new_capacity = heap->capacity == 0 ? 8 : heap->capacity * 2;
        if (new_capacity < heap->capacity)
            return -1;
        if (binary_heap_reserve(heap, new_capacity) != 0)
            return -1;
    }

    heap->data[heap->size] = item;
    binary_heap_sift_up(heap, heap->size);
    heap->size++;

    return 0;
}

static void *binary_heap_peek(const struct priority_queue *queue)
{
    const struct binary_heap *heap = binary_heap_from_const_queue(queue);

    if (heap->size == 0)
        return NULL;

    return heap->data[0];
}

static void *binary_heap_pop(struct priority_queue *queue)
{
    struct binary_heap *heap = binary_heap_from_queue(queue);
    void *item;

    if (heap->size == 0)
        return NULL;

    item = heap->data[0];
    heap->size--;

    if (heap->size > 0) {
        heap->data[0] = heap->data[heap->size];
        binary_heap_sift_down(heap, 0);
    }

    return item;
}

static size_t binary_heap_size(const struct priority_queue *queue)
{
    return binary_heap_from_const_queue(queue)->size;
}

static int binary_heap_empty(const struct priority_queue *queue)
{
    return binary_heap_size(queue) == 0;
}

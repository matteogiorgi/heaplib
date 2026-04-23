#ifndef PQLIB_HEAPS_BINARY_HEAP_H
#define PQLIB_HEAPS_BINARY_HEAP_H

#include "pqlib/priority_queue.h"

struct priority_queue *binary_heap_create(priority_queue_cmp_fn cmp);

#endif

#ifndef HPQLIB_HEAPS_KAPLAN_HEAP_H
#define HPQLIB_HEAPS_KAPLAN_HEAP_H

#include "hpqlib/priority_queue.h"

/*
 * Planned priority_queue backend based on Fibonacci heaps revisited, referred
 * to in this project as Kaplan heaps.
 *
 * The factory is wired into the public selector table so the final heap-only
 * architecture is visible, but the backend is not implemented yet.
 */
struct priority_queue *kaplan_heap_create(priority_queue_cmp_fn cmp);

#endif

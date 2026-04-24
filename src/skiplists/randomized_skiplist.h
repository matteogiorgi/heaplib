#ifndef PQLIB_SKIPLISTS_RANDOMIZED_SKIPLIST_H
#define PQLIB_SKIPLISTS_RANDOMIZED_SKIPLIST_H

#include "pqlib/priority_queue.h"

/*
 * Create a priority_queue backed by a randomized skiplist.
 *
 * This is a private factory used by the public priority_queue_create()
 * dispatcher. The returned object must be handled only through the abstract
 * priority_queue API.
 */
struct priority_queue *randomized_skiplist_create(priority_queue_cmp_fn cmp);

#endif

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <stddef.h>
#include <string.h>

#include "hpqlib/priority_queue.h"

/*
 * Python object that owns one native priority_queue instance.
 *
 * Items stored in the native queue are always PyObject pointers. The wrapper
 * owns one strong reference for each stored item until it is popped or the
 * queue object is destroyed.
 */
typedef struct PyPriorityQueueHandle PyPriorityQueueHandle;

typedef struct {
    PyObject_HEAD
    struct priority_queue *queue;
    PyPriorityQueueHandle *handles;
} PyPriorityQueue;

struct PyPriorityQueueHandle {
    PyObject_HEAD
    struct priority_queue_handle *handle;
    PyPriorityQueue *owner;
    PyObject *item;
    PyPriorityQueueHandle *previous;
    PyPriorityQueueHandle *next;
};

static PyTypeObject PyPriorityQueueHandleType;

static void PyPriorityQueueHandle_unlink(PyPriorityQueueHandle *handle)
{
    PyPriorityQueue *owner = handle->owner;

    if (owner == NULL)
        return;

    if (handle->previous != NULL)
        handle->previous->next = handle->next;
    else
        owner->handles = handle->next;

    if (handle->next != NULL)
        handle->next->previous = handle->previous;

    handle->previous = NULL;
    handle->next = NULL;
}

static void PyPriorityQueueHandle_register(
    PyPriorityQueue *owner,
    PyPriorityQueueHandle *handle
)
{
    handle->owner = owner;
    handle->previous = NULL;
    handle->next = owner->handles;

    if (owner->handles != NULL)
        owner->handles->previous = handle;

    owner->handles = handle;
}

static void PyPriorityQueueHandle_invalidate(PyPriorityQueueHandle *handle)
{
    PyPriorityQueueHandle_unlink(handle);
    handle->handle = NULL;
    handle->owner = NULL;
    handle->item = NULL;
}

static void PyPriorityQueue_invalidate_all_handles(PyPriorityQueue *self)
{
    while (self->handles != NULL)
        PyPriorityQueueHandle_invalidate(self->handles);
}

static void PyPriorityQueueHandle_init(
    PyPriorityQueueHandle *handle,
    struct priority_queue_handle *native_handle,
    PyPriorityQueue *owner,
    PyObject *item
)
{
    handle->handle = native_handle;
    handle->item = item;
    handle->previous = NULL;
    handle->next = NULL;
    PyPriorityQueueHandle_register(owner, handle);
}

static void PyPriorityQueue_invalidate_item_handles(
    PyPriorityQueue *self,
    PyObject *item
)
{
    PyPriorityQueueHandle *handle = self->handles;

    while (handle != NULL) {
        PyPriorityQueueHandle *next = handle->next;

        if (handle->owner == self && handle->item == item)
            PyPriorityQueueHandle_invalidate(handle);

        handle = next;
    }
}

static int PyPriorityQueueHandle_validate(
    PyPriorityQueue *self,
    PyObject *object,
    PyPriorityQueueHandle **handle
)
{
    if (!PyObject_TypeCheck(object, &PyPriorityQueueHandleType)) {
        PyErr_SetString(
            PyExc_TypeError,
            "expected a PriorityQueueHandle created by push_handle()"
        );
        return -1;
    }

    *handle = (PyPriorityQueueHandle *)object;
    if ((*handle)->owner != self || (*handle)->handle == NULL) {
        PyErr_SetString(PyExc_ValueError, "priority queue handle is invalid");
        return -1;
    }

    return 0;
}

/*
 * Release the native queue and any Python objects still stored in it.
 */
static void PyPriorityQueue_clear(PyPriorityQueue *self)
{
    if (self->queue != NULL) {
        PyObject *item;

        PyPriorityQueue_invalidate_all_handles(self);
        while ((item = priority_queue_pop(self->queue)) != NULL)
            Py_DECREF(item);

        priority_queue_destroy(self->queue);
        self->queue = NULL;
    }

    self->handles = NULL;
}

/*
 * Compare two Python objects using their natural ordering.
 *
 * The priority_queue contract expects a negative value when lhs has higher
 * priority. For Python objects, lower values are treated as higher priority.
 */
static int py_priority_queue_cmp(const void *lhs, const void *rhs)
{
    PyObject *left = (PyObject *) lhs;
    PyObject *right = (PyObject *) rhs;
    int result;

    result = PyObject_RichCompareBool(left, right, Py_LT);
    if (result < 0) {
        PyErr_Clear();
        return 0;
    }
    if (result)
        return -1;

    result = PyObject_RichCompareBool(left, right, Py_GT);
    if (result < 0) {
        PyErr_Clear();
        return 0;
    }
    if (result)
        return 1;

    return 0;
}

static int parse_implementation(
    const char *name,
    enum priority_queue_implementation *implementation
)
{
    if (strcmp(name, "binary_heap") == 0) {
        *implementation = PRIORITY_QUEUE_BINARY_HEAP;
        return 0;
    }

    if (strcmp(name, "fibonacci_heap") == 0) {
        *implementation = PRIORITY_QUEUE_FIBONACCI_HEAP;
        return 0;
    }

    if (strcmp(name, "kaplan_heap") == 0) {
        *implementation = PRIORITY_QUEUE_KAPLAN_HEAP;
        return 0;
    }

    PyErr_SetString(
        PyExc_ValueError,
        "implementation must be 'binary_heap', 'fibonacci_heap', or "
        "'kaplan_heap'"
    );
    return -1;
}

/*
 * Initialize a Python PriorityQueue instance.
 *
 * The optional implementation keyword maps stable Python-facing strings to the
 * public C implementation enum. Re-initialization is supported by first
 * releasing any native queue already owned by the object.
 */
static int PyPriorityQueue_init(
    PyPriorityQueue *self,
    PyObject *args,
    PyObject *kwargs
)
{
    static char *kwlist[] = { "implementation", NULL };
    const char *implementation_name = "binary_heap";
    enum priority_queue_implementation implementation;

    if (!PyArg_ParseTupleAndKeywords(
            args,
            kwargs,
            "|s",
            kwlist,
            &implementation_name
        ))
        return -1;

    if (parse_implementation(implementation_name, &implementation) != 0)
        return -1;

    PyPriorityQueue_clear(self);

    self->queue = priority_queue_create(implementation, py_priority_queue_cmp);
    if (self->queue == NULL) {
        PyErr_NoMemory();
        return -1;
    }

    return 0;
}

/*
 * Deallocate the Python wrapper and release all native resources.
 */
static void PyPriorityQueue_dealloc(PyPriorityQueue *self)
{
    PyPriorityQueue_clear(self);
    Py_TYPE(self)->tp_free((PyObject *) self);
}

/*
 * Store an item in the native queue.
 *
 * The wrapper takes one strong reference before handing the pointer to C. If
 * insertion fails, that reference is immediately released and MemoryError is
 * reported to Python.
 */
static PyObject *PyPriorityQueue_push(PyPriorityQueue *self, PyObject *item)
{
    Py_INCREF(item);
    if (priority_queue_push(self->queue, item) != 0) {
        Py_DECREF(item);
        return PyErr_NoMemory();
    }

    Py_RETURN_NONE;
}

/*
 * Store an item and return an opaque handle for targeted operations.
 */
static PyObject *PyPriorityQueue_push_handle(
    PyPriorityQueue *self,
    PyObject *item
)
{
    struct priority_queue_handle *native_handle;
    PyPriorityQueueHandle *handle;

    Py_INCREF(item);
    native_handle = priority_queue_push_handle(self->queue, item);
    if (native_handle == NULL) {
        Py_DECREF(item);
        return PyErr_NoMemory();
    }

    handle = PyObject_New(PyPriorityQueueHandle, &PyPriorityQueueHandleType);
    if (handle == NULL) {
        PyObject *removed = priority_queue_remove(self->queue, native_handle);

        Py_XDECREF(removed);
        return NULL;
    }

    PyPriorityQueueHandle_init(handle, native_handle, self, item);

    return (PyObject *)handle;
}

/*
 * Repair native heap order after the handled item's key has decreased.
 */
static PyObject *PyPriorityQueue_decrease_key(
    PyPriorityQueue *self,
    PyObject *object
)
{
    PyPriorityQueueHandle *handle;

    if (PyPriorityQueueHandle_validate(self, object, &handle) != 0)
        return NULL;

    if (priority_queue_decrease_key(self->queue, handle->handle) != 0) {
        PyErr_SetString(PyExc_RuntimeError, "decrease_key failed");
        return NULL;
    }

    Py_RETURN_NONE;
}

/*
 * Remove the item identified by a live handle.
 */
static PyObject *PyPriorityQueue_remove(
    PyPriorityQueue *self,
    PyObject *object
)
{
    PyPriorityQueueHandle *handle;
    PyObject *item;

    if (PyPriorityQueueHandle_validate(self, object, &handle) != 0)
        return NULL;

    item = priority_queue_remove(self->queue, handle->handle);
    if (item == NULL) {
        PyErr_SetString(PyExc_RuntimeError, "remove failed");
        return NULL;
    }

    PyPriorityQueueHandle_invalidate(handle);
    return item;
}

/*
 * Return whether the native queue stores item by pointer identity.
 */
static PyObject *PyPriorityQueue_contains(PyPriorityQueue *self, PyObject *item)
{
    if (priority_queue_contains(self->queue, item))
        Py_RETURN_TRUE;

    Py_RETURN_FALSE;
}

/*
 * Return the highest-priority item without removing it.
 *
 * The native queue retains ownership of its stored reference, so peek returns a
 * new reference to the same Python object.
 */
static PyObject *PyPriorityQueue_peek(
    PyPriorityQueue *self,
    PyObject *Py_UNUSED(ignored)
)
{
    PyObject *item = priority_queue_peek(self->queue);

    if (item == NULL)
        Py_RETURN_NONE;

    Py_INCREF(item);
    return item;
}

/*
 * Remove and return the highest-priority item.
 *
 * The stored strong reference is transferred to the caller as the return value.
 */
static PyObject *PyPriorityQueue_pop(
    PyPriorityQueue *self,
    PyObject *Py_UNUSED(ignored)
)
{
    PyObject *item = priority_queue_pop(self->queue);

    if (item == NULL)
        Py_RETURN_NONE;

    PyPriorityQueue_invalidate_item_handles(self, item);
    return item;
}

/*
 * Return the queue size as a Python integer.
 */
static PyObject *PyPriorityQueue_size(
    PyPriorityQueue *self,
    PyObject *Py_UNUSED(ignored)
)
{
    return PyLong_FromSize_t(priority_queue_size(self->queue));
}

/*
 * Return whether the queue is empty as a Python bool.
 */
static PyObject *PyPriorityQueue_empty(
    PyPriorityQueue *self,
    PyObject *Py_UNUSED(ignored)
)
{
    if (priority_queue_empty(self->queue))
        Py_RETURN_TRUE;

    Py_RETURN_FALSE;
}

/*
 * Implement len(queue).
 */
static Py_ssize_t PyPriorityQueue_len(PyPriorityQueue *self)
{
    size_t size = priority_queue_size(self->queue);

    if (size > (size_t) PY_SSIZE_T_MAX) {
        PyErr_SetString(PyExc_OverflowError, "priority queue is too large");
        return -1;
    }

    return (Py_ssize_t) size;
}

/*
 * Implement bool(queue).
 */
static int PyPriorityQueue_bool(PyPriorityQueue *self)
{
    return !priority_queue_empty(self->queue);
}

/*
 * Method table exposed on hpqlib.PriorityQueue.
 */
static PyMethodDef PyPriorityQueue_methods[] = {
    {
        "push",
        (PyCFunction) PyPriorityQueue_push,
        METH_O,
        "Insert an item into the priority queue."
    },
    {
        "push_handle",
        (PyCFunction) PyPriorityQueue_push_handle,
        METH_O,
        "Insert an item and return a handle for targeted operations."
    },
    {
        "decrease_key",
        (PyCFunction) PyPriorityQueue_decrease_key,
        METH_O,
        "Repair queue order after a handled item moves closer to the minimum."
    },
    {
        "remove",
        (PyCFunction) PyPriorityQueue_remove,
        METH_O,
        "Remove and return the item identified by a handle."
    },
    {
        "contains",
        (PyCFunction) PyPriorityQueue_contains,
        METH_O,
        "Return True if the exact object is stored in the priority queue."
    },
    {
        "peek",
        (PyCFunction) PyPriorityQueue_peek,
        METH_NOARGS,
        "Return the highest-priority item without removing it."
    },
    {
        "pop",
        (PyCFunction) PyPriorityQueue_pop,
        METH_NOARGS,
        "Remove and return the highest-priority item."
    },
    {
        "size",
        (PyCFunction) PyPriorityQueue_size,
        METH_NOARGS,
        "Return the number of stored items."
    },
    {
        "empty",
        (PyCFunction) PyPriorityQueue_empty,
        METH_NOARGS,
        "Return True when the priority queue is empty."
    },
    { NULL, NULL, 0, NULL }
};

/*
 * Python object representing one backend-owned priority_queue_handle.
 */
static void PyPriorityQueueHandle_dealloc(PyPriorityQueueHandle *self)
{
    PyPriorityQueueHandle_invalidate(self);
    Py_TYPE(self)->tp_free((PyObject *)self);
}

static PyTypeObject PyPriorityQueueHandleType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "hpqlib.PriorityQueueHandle",
    .tp_basicsize = sizeof(PyPriorityQueueHandle),
    .tp_itemsize = 0,
    .tp_dealloc = (destructor)PyPriorityQueueHandle_dealloc,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_doc = "Opaque handle returned by PriorityQueue.push_handle().",
};

/*
 * Sequence protocol support. Only sq_length is implemented.
 */
static PySequenceMethods PyPriorityQueue_sequence = {
    (lenfunc) PyPriorityQueue_len,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0
};

/*
 * Number protocol support for truth-value testing.
 */
static PyNumberMethods PyPriorityQueue_number = {
    .nb_bool = (inquiry) PyPriorityQueue_bool,
};

/*
 * Python type object for hpqlib.PriorityQueue.
 */
static PyTypeObject PyPriorityQueueType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "hpqlib.PriorityQueue",
    .tp_basicsize = sizeof(PyPriorityQueue),
    .tp_itemsize = 0,
    .tp_dealloc = (destructor) PyPriorityQueue_dealloc,
    .tp_as_number = &PyPriorityQueue_number,
    .tp_as_sequence = &PyPriorityQueue_sequence,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_doc = "PriorityQueue(implementation='binary_heap')",
    .tp_methods = PyPriorityQueue_methods,
    .tp_init = (initproc) PyPriorityQueue_init,
    .tp_new = PyType_GenericNew,
};

/*
 * Module definition for the single-extension hpqlib module.
 */
static PyModuleDef hpqlibmodule = {
    PyModuleDef_HEAD_INIT,
    .m_name = "hpqlib",
    .m_doc = "Python bindings for hpqlib priority queues.",
    .m_size = -1,
};

/*
 * Module initialization entry point.
 */
PyMODINIT_FUNC PyInit_hpqlib(void)
{
    PyObject *module;

    if (PyType_Ready(&PyPriorityQueueHandleType) < 0)
        return NULL;

    if (PyType_Ready(&PyPriorityQueueType) < 0)
        return NULL;

    module = PyModule_Create(&hpqlibmodule);
    if (module == NULL)
        return NULL;

    Py_INCREF(&PyPriorityQueueType);
    if (PyModule_AddObject(
            module,
            "PriorityQueue",
            (PyObject *) &PyPriorityQueueType
        ) < 0) {
        Py_DECREF(&PyPriorityQueueType);
        Py_DECREF(module);
        return NULL;
    }

    Py_INCREF(&PyPriorityQueueHandleType);
    if (PyModule_AddObject(
            module,
            "PriorityQueueHandle",
            (PyObject *)&PyPriorityQueueHandleType
        ) < 0) {
        Py_DECREF(&PyPriorityQueueHandleType);
        Py_DECREF(module);
        return NULL;
    }

    return module;
}

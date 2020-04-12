#define PY_SSIZE_T_CLEAN
#include "jswrapper.h"
#include <structmember.h>

typedef struct {
    PyObject_HEAD
    /* Type-specific fields go here. */
    void * wrapped_object;
} WrappedJSObject;

static void
WrappedJSObject_dealloc(WrappedJSObject *self)
{
    // TODO - decref wrapped_object
    Py_TYPE(self)->tp_free((PyObject *) self);
}

static PyObject *
WrappedJSObject_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
    WrappedJSObject *self;
    self = (WrappedJSObject *) type->tp_alloc(type, 0);
    if (self != NULL) {
        self->wrapped_object = NULL;
    }
    return (PyObject *) self;
}

static int
WrappedJSObject_init(WrappedJSObject *self, PyObject *args, PyObject *kwds)
{
    if (!PyArg_ParseTuple(args, ""))
        return -1;
    return 0;
}

static PyMemberDef WrappedJSObject_members[] = {
    {NULL}  /* Sentinel */
};

static PyObject *
WrappedJSObject_something(WrappedJSObject *self, PyObject *unused)
{
    //self->state++;
    //return PyLong_FromLong(self->state);
    return PyLong_FromLong(47.0);
}

static PyMethodDef WrappedJSObject_methods[] = {
    {"something", (PyCFunction) WrappedJSObject_something, METH_NOARGS,
     PyDoc_STR("Do something?")},
    {NULL}, /* Sentinel */
};

static PyTypeObject WrappedJSType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "pynode.WrappedJSObject",
    .tp_doc = "A JavaScript object",
    .tp_basicsize = sizeof(WrappedJSObject),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_new = WrappedJSObject_new,
    .tp_init = (initproc) WrappedJSObject_init,
    .tp_dealloc = (destructor) WrappedJSObject_dealloc,
    .tp_members = WrappedJSObject_members,
    .tp_methods = WrappedJSObject_methods,
};

PyObject *WrappedJSObject_New() {
    WrappedJSObject *obj = PyObject_NEW(WrappedJSObject, &WrappedJSType);
    printf("Created new object\n");
    return (PyObject *)obj;
}

static PyModuleDef pynodemodule = {
    PyModuleDef_HEAD_INIT,
    .m_name = "pynode",
    .m_doc = "Python <3 JavaScript.",
    .m_size = -1,
};

PyMODINIT_FUNC
PyInit_jswrapper(void)
{
    PyObject *m;
    if (PyType_Ready(&WrappedJSType) < 0)
        return NULL;

    m = PyModule_Create(&pynodemodule);
    if (m == NULL)
        return NULL;

    Py_INCREF(&WrappedJSType);
    if (PyModule_AddObject(m, "WrappedJSObject", (PyObject *) &WrappedJSType) < 0) {
        Py_DECREF(&WrappedJSType);
        Py_DECREF(m);
        return NULL;
    }

    return m;
}

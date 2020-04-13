#define PY_SSIZE_T_CLEAN
#include "jswrapper.h"
#include <structmember.h>

typedef struct {
    PyObject_HEAD
    /* Type-specific fields go here. */
    napi_ref object_reference;
    napi_env env;
} WrappedJSObject;

static void
WrappedJSObject_dealloc(WrappedJSObject *self)
{
    if (self->object_reference != NULL) {
        napi_delete_reference(self->env, self->object_reference);
        self->object_reference = NULL;
    }
    Py_TYPE(self)->tp_free((PyObject *) self);
}

static void
WrappedJSObject_assign_napi_value(WrappedJSObject *self, napi_env env, napi_value value) {
    self->env = env;
    if (self->object_reference != NULL) {
        napi_delete_reference(env, self->object_reference);
        self->object_reference = NULL;
    }
    napi_create_reference(env, value, 1, &(self->object_reference));
}

static PyObject *
WrappedJSObject_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
    WrappedJSObject *self;
    self = (WrappedJSObject *) type->tp_alloc(type, 0);
    if (self != NULL) {
        self->object_reference = NULL;
        self->env = NULL;
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

PyObject *WrappedJSObject_New(napi_env env, napi_value value) {
    /* Pass an empty argument list */
    PyObject *argList = Py_BuildValue("()");

    /* Call the class object. */
    PyObject *obj = PyObject_CallObject((PyObject *) &WrappedJSType, argList);
    if (obj == NULL) {
        PyErr_Print();
    }

    /* Release the argument list. */
    Py_DECREF(argList);

    WrappedJSObject_assign_napi_value((WrappedJSObject*)obj, env, value);
    return obj;
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

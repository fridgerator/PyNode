#define PY_SSIZE_T_CLEAN
#include "jswrapper.h"
#include <structmember.h>

typedef struct {
    PyObject_HEAD
    /* Type-specific fields go here. */
    napi_ref object_reference;
    napi_env env;
    napi_value bound;
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

napi_value WrappedJSObject_get_napi_value(PyObject *_self) {
    WrappedJSObject *self = (WrappedJSObject *)_self;
    napi_value wrapped;
    napi_get_reference_value(self->env, self->object_reference, &wrapped);
    return wrapped;
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
        self->bound = NULL;
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

static PyObject *
WrappedJSObject_getattro(PyObject *_self, PyObject *attr)
{
    WrappedJSObject *self = (WrappedJSObject*)_self;
    napi_value wrapped;
    napi_valuetype type;
    bool hasattr;
    const char * utf8name = PyUnicode_AsUTF8(attr);
    napi_value result;
    napi_get_reference_value(self->env, self->object_reference, &wrapped);
    napi_has_named_property(self->env, wrapped, utf8name, &hasattr);
    if (hasattr) {
        bool isfunc;
        napi_get_named_property(self->env, wrapped, utf8name, &result);
        napi_typeof(self->env, result, &type);
        isfunc = (type == napi_function);

        PyObject *pyval = convert_napi_value_to_python(self->env, result);
        if (pyval != NULL) {
            if (isfunc) {
                /* "bind" the method to its instance */
                ((WrappedJSObject *)pyval)->bound = wrapped;
            }
            return pyval;
        }
    }
    PyErr_SetObject(PyExc_AttributeError, attr);
    return NULL;
}

static PyObject *
WrappedJSObject_call(PyObject *_self, PyObject *args, PyObject *kwargs)
{
    WrappedJSObject *self = (WrappedJSObject*)_self;
    PyObject * ret = NULL;
    PyObject * seq;
    Py_ssize_t len = 0, i;
    napi_value result;
    napi_value this;
    napi_status status;
    napi_value wrapped;
    napi_value * jsargs = NULL;

    seq = PySequence_Fast(args, "*args must be a sequence");
    len = PySequence_Size(args);
    jsargs = calloc(len, sizeof(napi_value));
    if (jsargs == NULL) {
        PyErr_SetString(PyExc_MemoryError, "Out of memory allocating JS args array");
        goto finally;
    }
    for (i = 0; i < len; i++) {
        PyObject *arg = PySequence_Fast_GET_ITEM(seq, i);
        napi_value jsarg = convert_python_to_napi_value(self->env, arg);
        jsargs[i] = jsarg;
    }
    Py_DECREF(seq);

    napi_get_reference_value(self->env, self->object_reference, &wrapped);

    if (self->bound != NULL) {
        this = self->bound;
    } else {
        status = napi_get_global(self->env, &this);
        if (status != napi_ok) {
            PyErr_SetString(PyExc_RuntimeError, "Error getting JS global environment");
            goto finally;
        }
    }

    status = napi_call_function(self->env, this, wrapped, len, jsargs, &result);
    if (status != napi_ok) {
        PyErr_SetString(PyExc_RuntimeError, "Error calling javascript function");
        goto finally;
    }

    PyObject *pyval = convert_napi_value_to_python(self->env, result);
    if (pyval == NULL) {
        PyErr_SetString(PyExc_RuntimeError, "Error converting JS return value to Python");
        goto finally;
    }
    ret = pyval;

finally:
    if (jsargs != NULL) {
        free(jsargs);
        jsargs = NULL;
    }
    return ret;
}

PyObject * WrappedJSObject_str(PyObject *_self) {
    WrappedJSObject *self = (WrappedJSObject *)_self;
    napi_value global;
    napi_value result;
    napi_value wrapped;
    napi_status status;

    status = napi_get_global(self->env, &global);
    if (status != napi_ok) {
        PyErr_SetString(PyExc_RuntimeError, "Error getting JS global environment");
        Py_RETURN_NONE;
    }

    napi_get_reference_value(self->env, self->object_reference, &wrapped);

    status = napi_coerce_to_string(self->env, wrapped, &result);
    if (status != napi_ok) {
        PyErr_SetString(PyExc_RuntimeError, "Error coercing javascript value to string");
        Py_RETURN_NONE;
    }

    /* Result should just be a JavaScript string at this point */
    PyObject *pyval = convert_napi_value_to_python(self->env, result);
    if (pyval == NULL) {
        PyErr_SetString(PyExc_RuntimeError, "Error converting JavaScript ToString item to Python");
        Py_RETURN_NONE;
    }
    return pyval;
}

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
    .tp_call = WrappedJSObject_call,
    .tp_getattro = WrappedJSObject_getattro,
    .tp_str = WrappedJSObject_str,
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

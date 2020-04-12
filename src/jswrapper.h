#ifndef PYNODE_JSWRAPPER_HPP
#define PYNODE_JSWRAPPER_HPP

#include <Python.h>

#ifdef __cplusplus
extern "C" {
#endif

PyMODINIT_FUNC PyInit_jswrapper(void);
PyObject *WrappedJSObject_New(void);

#ifdef __cplusplus
}
#endif

#endif

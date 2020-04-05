#ifndef PYNODE_HELPERS_HPP
#define PYNODE_HELPERS_HPP

#include "napi.h"
#include "pywrapper.hpp"
#include <Python.h>

class py_context {
public:
  py_context() { gstate = PyGILState_Ensure(); }

  ~py_context() {
    PyGILState_Release(gstate);
    if (PyGILState_Check() == 1)
      pts = PyEval_SaveThread();
  }

private:
  PyGILState_STATE gstate;
  PyThreadState *pts;
};

// v8 to Python
PyObject *BuildPyArray(Napi::Env env, Napi::Value arg);
PyObject *BuildPyDict(Napi::Env env, Napi::Value arg);
PyObject *BuildPyArgs(const Napi::CallbackInfo &info, size_t start_index, size_t count);

// Python to v8
Napi::Array BuildV8Array(Napi::Env env, PyObject *obj);
Napi::Object BuildV8Dict(Napi::Env env, PyObject *obj);
Napi::Value ConvertFromPython(Napi::Env env, PyObject *obj);

int Py_GetNumArguments(PyObject *pFunc);

#endif

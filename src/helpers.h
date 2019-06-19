#include <Python.h>
#include <nan.h>

// v8 to Python
PyObject *BuildPyArray(v8::Local<v8::Value> arg);
PyObject *BuildPyArgs(const Nan::FunctionCallbackInfo<v8::Value> &args);

// Python to v8
v8::Local<v8::Array> BuildV8Array(PyObject *obj);

int Py_GetNumArguments(PyObject *pFunc);
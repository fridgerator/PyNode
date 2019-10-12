#include <Python.h>
#ifdef COMPILER
#undef COMPILER
#endif
#include <nan.h>

// v8 to Python
PyObject *BuildPyArray(v8::Local<v8::Value> arg);
PyObject *BuildPyDict(v8::Local<v8::Value> arg);
PyObject *BuildPyArgs(const v8::FunctionCallbackInfo<v8::Value>& args);

// Python to v8
v8::Local<v8::Array> BuildV8Array(PyObject *obj);
v8::Local<v8::Object> BuildV8Dict(PyObject *obj);

int Py_GetNumArguments(PyObject *pFunc);

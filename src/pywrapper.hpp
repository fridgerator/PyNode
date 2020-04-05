#ifndef PYNODE_PYWRAPPER_HPP
#define PYNODE_PYWRAPPER_HPP

#include "Python.h"
#include "helpers.hpp"
#include "napi.h"

class PyNodeWrappedPythonObject : public Napi::ObjectWrap<PyNodeWrappedPythonObject> {
  public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    PyNodeWrappedPythonObject(const Napi::CallbackInfo &info);
    static Napi::FunctionReference constructor;
    Napi::Value GetValue(const Napi::CallbackInfo &info);
    Napi::Value SetValue(const Napi::CallbackInfo &info);

  private:
    PyObject * _value;
};

#endif


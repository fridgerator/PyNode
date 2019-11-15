#include "worker.hpp"
#include <frameobject.h>
#include <iostream>
#include <sstream>

PyNodeWorker::PyNodeWorker(Napi::Function &callback, PyObject *pyArgs,
                           PyObject *pFunc)
    : Napi::AsyncWorker(callback), pyArgs(pyArgs), pFunc(pFunc){};
PyNodeWorker::~PyNodeWorker(){};

void PyNodeWorker::Execute() {
  {
    py_context ctx;

    pValue = PyObject_CallObject(pFunc, pyArgs);
    Py_DECREF(pyArgs);
    PyObject *errOccurred = PyErr_Occurred();

    if (errOccurred != NULL) {
      std::string error;
      PyObject *pType, *pValue, *pTraceback, *pTypeString;
      PyErr_Fetch(&pType, &pValue, &pTraceback);
      const char *value = PyUnicode_AsUTF8(pValue);
      pTypeString = PyObject_Str(pType);
      const char *type = PyUnicode_AsUTF8(pTypeString);
      PyTracebackObject *tb = (PyTracebackObject *)pTraceback;
      _frame *frame = tb->tb_frame;

      while (frame != NULL) {
        int line = PyCode_Addr2Line(frame->f_code, frame->f_lasti);
        const char *filename = PyUnicode_AsUTF8(frame->f_code->co_filename);
        const char *funcname = PyUnicode_AsUTF8(frame->f_code->co_name);
        error.append("File \"" + std::string(filename) + "\", line " +
                     std::to_string(line) + ", in " + std::string(funcname) +
                     "\n");
        error.append(std::string(type) + ": " + std::string(value));
        frame = frame->f_back;
      }

      Py_DecRef(errOccurred);
      Py_DecRef(pTypeString);
      PyErr_Restore(pType, pValue, pTraceback);
      PyErr_Print();
      SetError(error);
    }
  }
}

void PyNodeWorker::OnOK() {
  py_context ctx;
  Napi::HandleScope scope(Env());

  std::vector<Napi::Value> result = {Env().Null(), Env().Null()};

  if (pValue != NULL) {
    if (strcmp(pValue->ob_type->tp_name, "NoneType") == 0) {
      // leave as null
    } else if (strcmp(pValue->ob_type->tp_name, "bool") == 0) {
      bool b = PyObject_IsTrue(pValue);
      result[1] = Napi::Boolean::New(Env(), b);
    } else if (strcmp(pValue->ob_type->tp_name, "int") == 0) {
      double d = PyLong_AsDouble(pValue);
      result[1] = Napi::Number::New(Env(), d);
    } else if (strcmp(pValue->ob_type->tp_name, "float") == 0) {
      double d = PyFloat_AsDouble(pValue);
      result[1] = Napi::Number::New(Env(), d);
    } else if (strcmp(pValue->ob_type->tp_name, "bytes") == 0) {
      auto str = Napi::String::New(Env(), PyBytes_AsString(pValue));
      result[1] = str;
    } else if (strcmp(pValue->ob_type->tp_name, "str") == 0) {
      auto str = Napi::String::New(Env(), PyUnicode_AsUTF8(pValue));
      result[1] = str;
    } else if (strcmp(pValue->ob_type->tp_name, "list") == 0) {
      auto arr = BuildV8Array(Env(), pValue);
      result[1] = arr;
    } else if (strcmp(pValue->ob_type->tp_name, "dict") == 0) {
      auto obj = BuildV8Dict(Env(), pValue);
      result[1] = obj;
    } else {
      std::string errMsg =
          std::string("Unsupported type returned (") +
          pValue->ob_type->tp_name +
          std::string("), only pure Python types are supported.");
      result[0] = Napi::String::New(Env(), errMsg);
    }

    Py_DECREF(pValue);
  } else {
    std::string error;
    error.append("Function call failed");
    Py_DECREF(pFunc);
    PyErr_Print();

    result[0] = Napi::Error::New(Env(), error).Value();
  }
  Callback().Call({result[0], result[1]});
}

void PyNodeWorker::OnError(const Napi::Error &e) {
  Callback().Call({Napi::Error::New(Env(), e.what()).Value()});
}
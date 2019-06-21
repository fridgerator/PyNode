#include <Python.h>
// #include <datetime.h>
// #include <node.h>
#include <nan.h>
#include <string>
#include <time.h>

#include "helpers.h"

PyObject *pModule;

void Call(const Nan::FunctionCallbackInfo<v8::Value> &args)
{
  PyObject *pFunc, *pValue;

  v8::String::Utf8Value functionName(args[0]);
  pFunc = PyObject_GetAttrString(pModule, *functionName);

  if (pFunc && PyCallable_Check(pFunc))
  {
    const int pythonArgsCount = Py_GetNumArguments(pFunc);
    // printf("this function has %d arguments\n", pythonArgsCount);
    const int passedArgsCount = args.Length() - 1;
    // printf("you passed %d arguments\n", passedArgsCount);

    // Check if the passed args length matches the python function args length
    if (passedArgsCount != pythonArgsCount)
    {
      char buff[100];
      snprintf(buff, sizeof(buff), "The function '%s' has %d arguments, %d were passed", *functionName, pythonArgsCount, passedArgsCount);
      Nan::ThrowError(buff);
      return;
    }

    PyObject *pArgs = BuildPyArgs(args);
    pValue = PyObject_CallObject(pFunc, pArgs);
    Py_DECREF(pArgs);

    // printf("return type : %s\n", pValue->ob_type->tp_name);

    if (pValue != NULL)
    {
      if (strcmp(pValue->ob_type->tp_name, "int") == 0)
      {
        double result = PyLong_AsDouble(pValue);
        args.GetReturnValue().Set(Nan::New(result));
      }
      else if (strcmp(pValue->ob_type->tp_name, "float") == 0)
      {
        double result = PyFloat_AsDouble(pValue);
        args.GetReturnValue().Set(Nan::New(result));
      }
      else if (strcmp(pValue->ob_type->tp_name, "bytes") == 0)
      {
        auto str = Nan::New(PyBytes_AsString(pValue)).ToLocalChecked();
        args.GetReturnValue().Set(str);
      }
      else if (strcmp(pValue->ob_type->tp_name, "bool") == 0)
      {
        bool b = PyObject_IsTrue(pValue);
        args.GetReturnValue().Set(Nan::New(b));
      }
      else if (strcmp(pValue->ob_type->tp_name, "list") == 0)
      {
        auto arr = BuildV8Array(pValue);
        args.GetReturnValue().Set(arr);
      }
      else if (strcmp(pValue->ob_type->tp_name, "dict") == 0)
      {
        auto obj = BuildV8Dict(pValue);
        args.GetReturnValue().Set(obj);
      }
      Py_DECREF(pValue);
    }
    else
    {
      Py_DECREF(pFunc);
      PyErr_Print();
      fprintf(stderr, "Call failed\n");
      Nan::ThrowError("Function call failed");
    }
  }
  else
  {
    Py_DECREF(pFunc);
    PyErr_Print();
    Nan::ThrowError("Function call failed");
  }
}

void Initialize(v8::Local<v8::Object> exports)
{
  // Initialize Python
  std::wstring path(L"/usr/lib/python3.7:/usr/local/lib/python3.7/lib-dynload:/usr/local/lib/python3.7/site-packages");
  const wchar_t *stdlib = path.c_str();
  Py_SetPath(stdlib);

  exports->Set(
      Nan::New("call").ToLocalChecked(),
      Nan::New<v8::FunctionTemplate>(Call)->GetFunction());

  Py_Initialize();
  PyRun_SimpleString("import sys");
  PyRun_SimpleString("sys.path.append(\".\")");

  PyObject *pName;
  pName = PyUnicode_DecodeFSDefault("tools");
  pModule = PyImport_Import(pName);
  Py_DECREF(pName);

  if (pModule == NULL)
  {
    PyErr_Print();
    fprintf(stderr, "Failed to load \%s\"\n", "tools");
    Nan::ThrowError("Failed to load python module");
    return;
  }
}

NODE_MODULE(addon, Initialize);

#include <Python.h>
// #include <datetime.h>
#ifdef COMPILER
#undef COMPILER
#endif
#include <nan.h>
#include <string>
#include <time.h>
#ifndef _WIN32
#include <dlfcn.h>
#endif

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
    const int passedArgsCount = args.Length() - 1;

    // Check if the passed args length matches the python function args length
    if (passedArgsCount != pythonArgsCount)
    {
      char *error;
      size_t len = (size_t)snprintf(NULL, 0, "The function '%s' has %d arguments, %d were passed", *functionName, pythonArgsCount, passedArgsCount);
      error = (char *)malloc(len + 1);
      snprintf(error, len + 1, "The function '%s' has %d arguments, %d were passed", *functionName, pythonArgsCount, passedArgsCount);
      Nan::ThrowError(error);
      free(error);
      return;
    }

    PyObject *pArgs = BuildPyArgs(args);
    pValue = PyObject_CallObject(pFunc, pArgs);
    Py_DECREF(pArgs);

    // printf("return type : %s\n", pValue->ob_type->tp_name);

    if (pValue != NULL)
    {
      if (pValue == Py_None)
      {
        args.GetReturnValue().Set(Nan::Null());
      }
      else if (PyBool_Check(pValue))
      {
        // because booleans are subtypes of integers this check must
        // come before PyLong_Check (ie, PyBool_Type is always a PyLong_Type but,
        // a PyLong_Type is not necessarily a PyBool_Type)
        bool b = PyObject_IsTrue(pValue);
        args.GetReturnValue().Set(Nan::New(b));
      }
      else if (PyLong_Check(pValue))
      {
        double result = PyLong_AsDouble(pValue);
        args.GetReturnValue().Set(Nan::New(result));
      }
      else if (PyFloat_Check(pValue))
      {
        double result = PyFloat_AsDouble(pValue);
        args.GetReturnValue().Set(Nan::New(result));
      }
      else if (PyBytes_Check(pValue))
      {
        auto str = Nan::New(PyBytes_AsString(pValue)).ToLocalChecked();
        args.GetReturnValue().Set(str);
      }
      else if (PyUnicode_Check(pValue))
      {
        auto str = Nan::New(PyUnicode_AsUTF8(pValue)).ToLocalChecked();
        args.GetReturnValue().Set(str);
      }
      else if (PyList_Check(pValue) || PyTuple_Check(pValue))
      {
        auto arr = BuildV8Array(pValue);
        args.GetReturnValue().Set(arr);
      }
      else if (PyDict_Check(pValue))
      {
        auto obj = BuildV8Dict(pValue);
        args.GetReturnValue().Set(obj);
      } 
      else
      {
        std::string errMsg = std::string("Unsupported type returned (") + pValue->ob_type->tp_name + std::string("), only pure Python types are supported.");
        Py_DECREF(pValue);
        return Nan::ThrowTypeError(Nan::New(errMsg).ToLocalChecked());
      }
      Py_DECREF(pValue);
    }
    // else
    // {
    //   Py_DECREF(pFunc);
    //   PyErr_Print();
    //   fprintf(stderr, "Call failed\n");
    //   Nan::ThrowError("Function call failed");
    // }
  }
  else
  {
    Py_DECREF(pFunc);
    PyErr_Print();
    Nan::ThrowError("Function call failed");
  }
}

void DLOpen(const Nan::FunctionCallbackInfo<v8::Value> &args)
{
  #ifdef _WIN32
    Nan::ThrowError("dlOpen does not work in windows");
    return;
  #endif

  if (args.Length() != 1 && !args[0]->IsString()) {
    Nan::ThrowError("Must pass a string to 'dlOpen'");
    return;
  }

  v8::String::Utf8Value dlFile(args[0]);
  #ifndef _WIN32
  dlopen(*dlFile, RTLD_LAZY | RTLD_GLOBAL);
  #endif
}

void StartInterpreter(const Nan::FunctionCallbackInfo<v8::Value> &args)
{
  if (args.Length() == 1 && args[0]->IsString()) {
    v8::String::Utf8Value pathString(args[0]);
    std::wstring path(pathString.length(), L'#');
    mbstowcs(&path[0], *pathString, pathString.length());
    Py_SetPath(path.c_str());
  }

  Py_Initialize();
}

void StopInterpreter(const Nan::FunctionCallbackInfo<v8::Value> &args)
{
  auto isInitialized = Py_IsInitialized();
  if (isInitialized == 0) return;
  Py_Finalize();
  Py_DECREF(pModule);
  pModule = NULL;
}

void AppendSysPath(const Nan::FunctionCallbackInfo<v8::Value> &args)
{
  if (args.Length() == 0 || !args[0]->IsString()) {
    Nan::ThrowError("Must pass a string to 'appendSysPath'");
    return;
  }

  v8::String::Utf8Value pathName(args[0]);

  char *appendPathStr;
  size_t len = (size_t)snprintf(NULL, 0, "import sys;sys.path.append(r\"%s\")", *pathName);
  appendPathStr = (char *)malloc(len + 1);
  snprintf(appendPathStr, len + 1, "import sys;sys.path.append(r\"%s\")", *pathName);

  PyRun_SimpleString(appendPathStr);
  free(appendPathStr);
}

void OpenFile(const Nan::FunctionCallbackInfo<v8::Value> &args)
{
  if (args.Length() == 0 || !args[0]->IsString()) {
    Nan::ThrowError("Must pass a string to 'openFile'");
    return;
  }

  v8::String::Utf8Value fileName(args[0]);

  PyObject *pName;
  pName = PyUnicode_DecodeFSDefault(*fileName);

  pModule = PyImport_Import(pName);
  Py_DECREF(pName);

  if (pModule == NULL)
  {
    PyErr_Print();
    fprintf(stderr, "Failed to load module: %s\n", *fileName);
    Nan::ThrowError("Failed to load python module");
    return;
  }
}

void Eval(const Nan::FunctionCallbackInfo<v8::Value> &args)
{
  if (args.Length() == 0 || !args[0]->IsString()) {
    Nan::ThrowError("Must pass a string to 'eval'");
    return;
  }

  v8::String::Utf8Value statement(args[0]);
  int response = PyRun_SimpleString(*statement);
  args.GetReturnValue().Set(Nan::New(response));
}

void Initialize(v8::Local<v8::Object> exports)
{
  exports->Set(
      Nan::New("call").ToLocalChecked(),
      Nan::New<v8::FunctionTemplate>(Call)->GetFunction());

  exports->Set(
      Nan::New("dlOpen").ToLocalChecked(),
      Nan::New<v8::FunctionTemplate>(DLOpen)->GetFunction());

  exports->Set(
      Nan::New("startInterpreter").ToLocalChecked(),
      Nan::New<v8::FunctionTemplate>(StartInterpreter)->GetFunction());

  exports->Set(
      Nan::New("stopInterpreter").ToLocalChecked(),
      Nan::New<v8::FunctionTemplate>(StopInterpreter)->GetFunction());

  exports->Set(
      Nan::New("appendSysPath").ToLocalChecked(),
      Nan::New<v8::FunctionTemplate>(AppendSysPath)->GetFunction());

  exports->Set(
      Nan::New("openFile").ToLocalChecked(),
      Nan::New<v8::FunctionTemplate>(OpenFile)->GetFunction());

  exports->Set(
      Nan::New("eval").ToLocalChecked(),
      Nan::New<v8::FunctionTemplate>(Eval)->GetFunction());
}

extern "C" NODE_MODULE_EXPORT void
NODE_MODULE_INITIALIZER(v8::Local<v8::Object> exports,
                        v8::Local<v8::Value> module,
                        v8::Local<v8::Context> context) {
  Initialize(exports);
}

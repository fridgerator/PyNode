#include "pynode.hpp"
#ifndef _WIN32
#include <dlfcn.h>
#endif
#include "helpers.hpp"
#include "worker.hpp"
#include "pywrapper.hpp"
#include "jswrapper.h"
#include <iostream>

PyObject *pModule;

Napi::Value StartInterpreter(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  if (info.Length() == 1 && info[0].IsString()) {
    std::string pathString = info[0].As<Napi::String>().ToString();
    std::wstring path(pathString.length(), L'#');
    mbstowcs(&path[0], pathString.c_str(), pathString.length());
    Py_SetPath(path.c_str());
  }
  
  PyImport_AppendInittab("pynode", &PyInit_jswrapper);

  int isInitialized = Py_IsInitialized();
  if (isInitialized == 0) {
    Py_Initialize();
  }

  int threadsInitialized = PyEval_ThreadsInitialized();
  if (threadsInitialized == 0)
    PyEval_InitThreads();

  /* Load PyNode's own module into Python. This makes WrappedJSObject instances
     behave better (eg, having attributes) */
  PyObject *pName;
  pName = PyUnicode_DecodeFSDefault("pynode");
  pModule = PyImport_Import(pName);
  Py_DECREF(pName);
  if (pModule == NULL) {
    PyErr_Print();
    Napi::Error::New(env, "Failed to load the pynode module into the Python interpreter")
        .ThrowAsJavaScriptException();
  }

  /* Release the GIL. The other entry points back into Python re-acquire it */
  PyEval_SaveThread();

  return env.Null();
}

Napi::Value DlOpen(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

#ifdef _WIN32
  Napi::Error::New(env, "dlOpen does not work in windows")
      .ThrowAsJavaScriptException();
  return env.Null();
#endif

  if (!info[0] || !info[0].IsString()) {
    Napi::Error::New(env, "Must pass a string to 'dlOpen'")
        .ThrowAsJavaScriptException();
    return env.Null();
  }

  std::string dlFile = info[0].As<Napi::String>().ToString();
#ifndef _WIN32
  dlopen(dlFile.c_str(), RTLD_LAZY | RTLD_GLOBAL);
#endif
  return env.Null();
}

Napi::Value AppendSysPath(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  if (!info[0] || !info[0].IsString()) {
    Napi::Error::New(env, "Must pass a string to 'appendSysPath'")
        .ThrowAsJavaScriptException();
    return env.Null();
  }

  std::string pathName = info[0].As<Napi::String>().ToString();
  char *appendPathStr;
  size_t len = (size_t)snprintf(NULL, 0, "import sys;sys.path.append(r\"%s\")",
                                pathName.c_str());
  appendPathStr = (char *)malloc(len + 1);
  snprintf(appendPathStr, len + 1, "import sys;sys.path.append(r\"%s\")",
           pathName.c_str());

  {
    py_ensure_gil ctx;
    PyRun_SimpleString(appendPathStr);
    free(appendPathStr);
  }

  return env.Null();
}

Napi::Value OpenFile(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  if (!info[0] || !info[0].IsString()) {
    Napi::Error::New(env, "Must pass a string to 'openFile'")
        .ThrowAsJavaScriptException();
    return env.Null();
  }

  std::string fileName = info[0].As<Napi::String>().ToString();

  {
    py_ensure_gil ctx;

    PyObject *pName;
    pName = PyUnicode_DecodeFSDefault(fileName.c_str());
    pModule = PyImport_Import(pName);
    Py_DECREF(pName);

    if (pModule == NULL) {
      PyErr_Print();
      std::cerr << "Failed to load module: " << fileName << std::endl;
      Napi::Error::New(env, "Failed to load python module")
          .ThrowAsJavaScriptException();
    }
  }

  return env.Null();
}

Napi::Value ImportModule(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  if (!info[0] || !info[0].IsString()) {
    Napi::Error::New(env, "Must pass a string to 'import'")
        .ThrowAsJavaScriptException();
    return env.Null();
  }

  std::string moduleName = info[0].As<Napi::String>().ToString();

  {
    py_ensure_gil ctx;

    PyObject *pName;
    pName = PyUnicode_FromString(moduleName.c_str());
    PyObject *module_object = PyImport_Import(pName);
    Py_DECREF(pName);

    if (module_object == NULL) {
      PyErr_Print();
      std::cerr << "Failed to load module: " << moduleName << std::endl;
      Napi::Error::New(env, "Failed to load python module")
          .ThrowAsJavaScriptException();
    }
    return ConvertFromPython(env, module_object);
  }

  return env.Null();
}

Napi::Value Eval(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  if (!info[0] || !info[0].IsString()) {
    Napi::TypeError::New(env, "Must pass a string to 'eval'")
        .ThrowAsJavaScriptException();
    return env.Null();
  }

  std::string statement = info[0].As<Napi::String>().ToString();
  int response;
  {
    py_ensure_gil ctx;

    response = PyRun_SimpleString(statement.c_str());
  }

  return Napi::Number::New(env, response);
}

Napi::Value Call(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  if (info.Length() == 0 || !info[0].IsString()) {
    std::cerr << "First argument to 'call' must be a string" << std::endl;
    Napi::Error::New(env, "First argument to 'call' must be a string")
        .ThrowAsJavaScriptException();
    return env.Null();
  }

  if (!info[info.Length() - 1].IsFunction()) {
    std::cerr << "Last argument to 'call' must be a function" << std::endl;
    Napi::Error::New(env, "Last argument to 'call' must be a function")
        .ThrowAsJavaScriptException();
    return env.Null();
  }

  std::string functionName = info[0].As<Napi::String>().ToString();

  PyObject *pFunc, *pArgs;

  {
    py_ensure_gil ctx;

    pFunc = PyObject_GetAttrString(pModule, functionName.c_str());
    int callable = PyCallable_Check(pFunc);

    if (pFunc != NULL && callable == 1) {
      const int pythonArgsCount = Py_GetNumArguments(pFunc);
      const int passedArgsCount = info.Length() - 2;

      // Check if the passed args length matches the python function args length
      if (passedArgsCount != pythonArgsCount) {
        std::string error("The function '" + functionName + "' has " +
                          std::to_string(pythonArgsCount) + " arguments, " +
                          std::to_string(passedArgsCount) + " were passed");
        Napi::Error::New(env, error).ThrowAsJavaScriptException();
        return env.Null();
      }

      // Arguments length minus 2: one for function name, one for js callback
      pArgs = BuildPyArgs(info, 1, info.Length() - 2);
    } else {
      Napi::Error::New(env,
                       "Could not find function name / function not callable")
          .ThrowAsJavaScriptException();
      return env.Null();
    }
  }

  Napi::Function cb = info[info.Length() - 1].As<Napi::Function>();
  PyNodeWorker *pnw = new PyNodeWorker(cb, pArgs, pFunc);
  pnw->Queue();
  return env.Null();
}

Napi::Object PyNodeInit(Napi::Env env, Napi::Object exports) {

  exports.Set(Napi::String::New(env, "dlOpen"),
              Napi::Function::New(env, DlOpen));

  exports.Set(Napi::String::New(env, "startInterpreter"),
              Napi::Function::New(env, StartInterpreter));

  exports.Set(Napi::String::New(env, "dlOpen"),
              Napi::Function::New(env, DlOpen));

  exports.Set(Napi::String::New(env, "appendSysPath"),
              Napi::Function::New(env, AppendSysPath));

  exports.Set(Napi::String::New(env, "openFile"),
              Napi::Function::New(env, OpenFile));

  exports.Set(Napi::String::New(env, "import"),
              Napi::Function::New(env, ImportModule));

  exports.Set(Napi::String::New(env, "eval"), Napi::Function::New(env, Eval));

  exports.Set(Napi::String::New(env, "call"), Napi::Function::New(env, Call));

  PyNodeWrappedPythonObject::Init(env, exports);

  return exports;
}

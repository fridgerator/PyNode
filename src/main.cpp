#include <sstream>
#include <Python.h>
#include <frameobject.h>
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

class PyNodeData {
  public:
    PyNodeData(v8::Isolate* isolate, v8::Local<v8::Object> exports):
      pModule(NULL) {
        exports_.Reset(isolate, exports);
        exports_.SetWeak(this, DeleteMe, v8::WeakCallbackType::kParameter);
      }

  PyObject *pModule;

  private:
    static void DeleteMe(const v8::WeakCallbackInfo<PyNodeData>& info) {
      delete info.GetParameter();
      fprintf(stderr, "delete me\n");
    }

    v8::Persistent<v8::Object> exports_;
};

void dlOpen(const Nan::FunctionCallbackInfo<v8::Value> &args)
{
  #ifdef _WIN32
    Nan::ThrowError("dlOpen does not work in windows");
    return;
  #endif

  if (args.Length() != 1 && !args[0]->IsString()) {
    Nan::ThrowError("Must pass a string to 'dlOpen'");
    return;
  }

  Nan::Utf8String dlFile(args[0]);
  #ifndef _WIN32
  dlopen(*dlFile, RTLD_LAZY | RTLD_GLOBAL);
  #endif
}

void startInterpreter(const v8::FunctionCallbackInfo<v8::Value>& info)
{
  if (info.Length() == 1 && info[0]->IsString()) {
    Nan::Utf8String pathString(info[0]);
    std::wstring path(pathString.length(), L'#');
    mbstowcs(&path[0], *pathString, pathString.length());
  }

  int isInitialized = Py_IsInitialized();
  if (isInitialized == 0) Py_Initialize();
}

void stopInterpreter(const v8::FunctionCallbackInfo<v8::Value>& info)
{
  PyNodeData* data =
      reinterpret_cast<PyNodeData*>(info.Data().As<v8::External>()->Value());

  auto isInitialized = Py_IsInitialized();
  if (isInitialized == 0) return;
  Py_Finalize();
  Py_DECREF(data->pModule);
  data->pModule = NULL;
}

void appendSysPath(const Nan::FunctionCallbackInfo<v8::Value> &args)
{
  if (args.Length() == 0 || !args[0]->IsString()) {
    Nan::ThrowError("Must pass a string to 'appendSysPath'");
    return;
  }

  Nan::Utf8String pathName(args[0]);

  char *appendPathStr;
  size_t len = (size_t)snprintf(NULL, 0, "import sys;sys.path.append(r\"%s\")", *pathName);
  appendPathStr = (char *)malloc(len + 1);
  snprintf(appendPathStr, len + 1, "import sys;sys.path.append(r\"%s\")", *pathName);

  PyGILState_STATE gstate;
  gstate = PyGILState_Ensure();
  PyRun_SimpleString(appendPathStr);
  free(appendPathStr);
  PyGILState_Release(gstate);
}

void openFile(const v8::FunctionCallbackInfo<v8::Value>& info)
{
  if (info.Length() == 0 || !info[0]->IsString()) {
    Nan::ThrowError("Must pass a string to 'openFile'");
    return;
  }

  PyNodeData* data =
      reinterpret_cast<PyNodeData*>(info.Data().As<v8::External>()->Value());

  Nan::Utf8String fileName(info[0]);

  PyGILState_STATE gstate;
  gstate = PyGILState_Ensure();

  PyObject *pName;
  pName = PyUnicode_DecodeFSDefault(*fileName);

  data->pModule = PyImport_Import(pName);
  Py_DECREF(pName);

  if (data->pModule == NULL)
  {
    PyErr_Print();
    fprintf(stderr, "Failed to load module: %s\n", *fileName);
    Nan::ThrowError("Failed to load python module");
    return;
  }

  PyGILState_Release(gstate);
}

void eval(const Nan::FunctionCallbackInfo<v8::Value> &args)
{
  if (args.Length() == 0 || !args[0]->IsString()) {
    Nan::ThrowError("Must pass a string to 'eval'");
    return;
  }

  Nan::Utf8String statement(args[0]);
  int response = PyRun_SimpleString(*statement);
  args.GetReturnValue().Set(Nan::New(response));
}

class CallWorker : public Nan::AsyncWorker {
  public:
    CallWorker(Nan::Callback *callback, PyObject *pyArgs, PyObject *pFunc)
      : Nan::AsyncWorker(callback), pyArgs(pyArgs), pFunc(pFunc) {
        gstate = PyGILState_Ensure();
      }
    ~CallWorker() {
      PyGILState_Release(gstate);
    }
    
    void Execute () {
      // Nan::HandleScope scope;
    }

    void HandleErrorCallback () {
      Nan::HandleScope scope;

      v8::Local<v8::Value> err[] = {
        v8::Exception::Error(Nan::New<v8::String>(ErrorMessage()).ToLocalChecked())
      };

      Nan::Call(callback->GetFunction(), Nan::GetCurrentContext()->Global(), 1, err);
    }

    void HandleOKCallback () {
      Nan::HandleScope scope;

      PyObject *pValue = PyObject_CallObject(pFunc, pyArgs);
      Py_DECREF(pyArgs);

      v8::Local<v8::Value> argv[] = {
        Nan::Null(),
        Nan::Null()
      };

      if (pValue != NULL) {

        if (strcmp(pValue->ob_type->tp_name, "NoneType") == 0)
        {
          // args.GetReturnValue().Set(Nan::Null());
        }
        else if (strcmp(pValue->ob_type->tp_name, "bool") == 0)
        {
          // because booleans are subtypes of integers this check must
          // come before PyLong_Check (ie, PyBool_Type is always a PyLong_Type but,
          // a PyLong_Type is not necessarily a PyBool_Type)
          bool b = PyObject_IsTrue(pValue);
          argv[1] = Nan::New(b);
        }
        else if (strcmp(pValue->ob_type->tp_name, "int") == 0)
        {
          double result = PyLong_AsDouble(pValue);
          argv[1] = Nan::New(result);
        }
        else if (strcmp(pValue->ob_type->tp_name, "float") == 0)
        {
          double result = PyFloat_AsDouble(pValue);
          argv[1] = Nan::New(result);
        }
        else if (strcmp(pValue->ob_type->tp_name, "bytes") == 0)
        {
          auto str = Nan::New(PyBytes_AsString(pValue)).ToLocalChecked();
          argv[1] = str;
        }
        else if (strcmp(pValue->ob_type->tp_name, "str") == 0)
        {
          auto str = Nan::New(PyUnicode_AsUTF8(pValue)).ToLocalChecked();
          argv[1] = str;
        }
        else if (strcmp(pValue->ob_type->tp_name, "list") == 0)
        {
          auto arr = BuildV8Array(pValue);
          argv[1] = arr;
        }
        else if (strcmp(pValue->ob_type->tp_name, "dict") == 0)
        {
          auto obj = BuildV8Dict(pValue);
          argv[1] = obj;
        }
        // else if (strcmp(pValue->ob_type->tp_name, "tuple") == 0)
        // {
        //   auto arr = BuildV8Array(pValue);
        //   args.GetReturnValue().Set(arr);
        // }
        else
        {
          std::string errMsg = std::string("Unsupported type returned (") + pValue->ob_type->tp_name + std::string("), only pure Python types are supported.");
          Py_DECREF(pValue);
          argv[0] = Nan::Error(errMsg.c_str());
        }

        Py_DECREF(pValue);
      } else {
        std::string error;
        PyObject * errOccurred = PyErr_Occurred();
        if (errOccurred != NULL) {
          PyObject *pType, *pValue, *pTraceback, *pTypeString;
          PyErr_Fetch(&pType, &pValue, &pTraceback);
          const char * value = PyUnicode_AsUTF8(pValue);
          pTypeString = PyObject_Str(pType);
          const char * type = PyUnicode_AsUTF8(pTypeString);

          PyTracebackObject * tb = (PyTracebackObject *)pTraceback;
          std::ostringstream stream;
          _frame * frame = tb->tb_frame;
          
          while (frame != NULL) {
            int line = PyCode_Addr2Line(frame->f_code, frame->f_lasti);
            const char * filename = PyUnicode_AsUTF8(frame->f_code->co_filename);
            const char * funcname = PyUnicode_AsUTF8(frame->f_code->co_name);
            stream << "File \"" << filename << "\", line " << line << ", in " << funcname << "\n";
            stream << type << ": " << value;
            frame = frame->f_back;
          }

          error.append(stream.str());

          Py_DecRef(errOccurred);
          Py_DecRef(pTypeString);
          PyErr_Restore(pType, pValue, pTraceback);
        } else {
          error.append("Function call failed");
        }

        Py_DecRef(pFunc);
        PyErr_Print();
        
        argv[0] = Nan::Error(error.c_str());
      }

      // PyGILState_Release(gstate);
      callback->Call(2, argv);
    }
  
  private:
    PyObject *pyArgs;
    PyObject *pFunc;
    PyGILState_STATE gstate;
};

void call(const v8::FunctionCallbackInfo<v8::Value>& info) {
  if (info.Length() == 0 || !info[0]->IsString()) {
    Nan::ThrowError("First argument to 'call' must be a string");
    return;
  }

  if (!info[info.Length() - 1]->IsFunction()) {
    Nan::ThrowError("Last argument to 'call' must be a function");
    return;
  }

  Nan::Utf8String functionName(info[0]);

  PyGILState_STATE gstate;
  gstate = PyGILState_Ensure();

  PyNodeData* data =
      reinterpret_cast<PyNodeData*>(info.Data().As<v8::External>()->Value());

  PyObject *pFunc, *pArgs;

  pFunc = PyObject_GetAttrString(data->pModule, *functionName);
  int callable = PyCallable_Check(pFunc);

  if (pFunc != NULL && callable == 1)
  {
    const int pythonArgsCount = Py_GetNumArguments(pFunc);
    const int passedArgsCount = info.Length() - 2;

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

    pArgs = BuildPyArgs(info);
  } else {
    Nan::ThrowError("Could not find function name / function not callable");
  }

  PyGILState_Release(gstate);

  Nan::AsyncQueueWorker(new CallWorker(
    new Nan::Callback(Nan::To<v8::Function>(info[info.Length() - 1]).ToLocalChecked()),
    pArgs,
    pFunc
  ));
}

extern "C" NODE_MODULE_EXPORT void
NODE_MODULE_INITIALIZER(v8::Local<v8::Object> exports,
                        v8::Local<v8::Value> module,
                        v8::Local<v8::Context> context) {
  v8::Isolate* isolate = context->GetIsolate();

  PyNodeData *data = new PyNodeData(isolate, exports);
  v8::Local<v8::External> external = v8::External::New(isolate, data);

  NAN_EXPORT(exports, dlOpen);
  NAN_EXPORT(exports, appendSysPath);
  NAN_EXPORT(exports, eval);

  exports->Set(context,
               v8::String::NewFromUtf8(isolate, "call", v8::NewStringType::kNormal)
                  .ToLocalChecked(),
               v8::FunctionTemplate::New(isolate, call, external)
                  ->GetFunction(context).ToLocalChecked()).FromJust();

  exports->Set(context,
               v8::String::NewFromUtf8(isolate, "startInterpreter", v8::NewStringType::kNormal)
                  .ToLocalChecked(),
               v8::FunctionTemplate::New(isolate, startInterpreter, external)
                  ->GetFunction(context).ToLocalChecked()).FromJust();

  exports->Set(context,
               v8::String::NewFromUtf8(isolate, "stopInterpreter", v8::NewStringType::kNormal)
                  .ToLocalChecked(),
               v8::FunctionTemplate::New(isolate, stopInterpreter, external)
                  ->GetFunction(context).ToLocalChecked()).FromJust();
  
  exports->Set(context,
               v8::String::NewFromUtf8(isolate, "openFile", v8::NewStringType::kNormal)
                  .ToLocalChecked(),
               v8::FunctionTemplate::New(isolate, openFile, external)
                  ->GetFunction(context).ToLocalChecked()).FromJust();
}

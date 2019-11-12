#include <Python.h>
#ifdef COMPILER
#undef COMPILER
#endif
#include <nan.h>

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

class PyNodeData {
public:
  PyNodeData(v8::Isolate *isolate, v8::Local<v8::Object> exports);

  PyObject *pModule;

private:
  static void DeleteMe(const v8::WeakCallbackInfo<PyNodeData> &info);

  v8::Persistent<v8::Object> exports_;
};

class CallWorker : public Nan::AsyncWorker {
public:
  CallWorker(Nan::Callback *callback, PyObject *pyArgs, PyObject *pFunc);
  ~CallWorker();
  void Execute();
  void HandleErrorCallback();
  void HandleOKCallback();

private:
  PyObject *pyArgs;
  PyObject *pFunc;
};

void dlOpen(const Nan::FunctionCallbackInfo<v8::Value> &args);
void startInterpreter(const v8::FunctionCallbackInfo<v8::Value> &info);
void appendSysPath(const Nan::FunctionCallbackInfo<v8::Value> &args);
void openFile(const v8::FunctionCallbackInfo<v8::Value> &info);
void eval(const Nan::FunctionCallbackInfo<v8::Value> &args);
void call(const v8::FunctionCallbackInfo<v8::Value> &info);

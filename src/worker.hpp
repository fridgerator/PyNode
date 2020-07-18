#ifndef PYNODE_WORKER_HPP
#define PYNODE_WORKER_HPP

#include "Python.h"
#include "pywrapper.hpp"
#include "helpers.hpp"
#include "napi.h"

class PyNodeWorker : public Napi::AsyncWorker {
public:
  PyNodeWorker(Napi::Function &callback, PyObject *pyArgs, PyObject *pFunc);
  ~PyNodeWorker();
  void Execute();
  void OnOK();
  void OnError(const Napi::Error &e);

private:
  PyObject *pyArgs;
  PyObject *pFunc;
  PyObject *pValue;
};

#endif

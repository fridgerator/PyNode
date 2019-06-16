#include <Python.h>
#include <node.h>
#include <nan.h>
#include <string>

PyObject *pModule;

void Multiply(const Nan::FunctionCallbackInfo<v8::Value> &args)
{
  if (!args[0]->IsNumber() || !args[1]->IsNumber())
  {
    Nan::ThrowError("Arguments must be a number");
    return;
  }

  PyObject *pFunc, *pArgs, *pValue;

  double a = Nan::To<double>(args[0]).FromJust();
  double b = Nan::To<double>(args[1]).FromJust();

  pFunc = PyObject_GetAttrString(pModule, "multiply");
  if (pFunc && PyCallable_Check(pFunc))
  {
    pArgs = PyTuple_New(2);
    PyTuple_SetItem(pArgs, 0, PyLong_FromLong(a));
    PyTuple_SetItem(pArgs, 1, PyLong_FromLong(b));

    pValue = PyObject_CallObject(pFunc, pArgs);
    Py_DECREF(pArgs);

    if (pValue != NULL)
    {
      long result = PyLong_AsLong(pValue);
      Py_DECREF(pValue);

      args.GetReturnValue().Set(Nan::New((double)result));
    }
    else
    {
      Py_DECREF(pFunc);
      PyErr_Print();
      fprintf(stderr, "Call failed\n");
      Nan::ThrowError("Function call failed");
    }
  }
}

void Initialize(v8::Local<v8::Object> exports)
{
  // Initialize Python
  std::wstring path(L"/usr/lib/python3.7:/usr/local/lib/python3.7/lib-dynload:/usr/local/lib/python3.7/site-packages");
  const wchar_t *stdlib = path.c_str();
  Py_SetPath(stdlib);

  exports->Set(
      Nan::New("multiply").ToLocalChecked(),
      Nan::New<v8::FunctionTemplate>(Multiply)->GetFunction());

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

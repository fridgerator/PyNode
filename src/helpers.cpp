#include "helpers.h"

PyObject *BuildPyDict(v8::Local<v8::Value> arg)
{
  auto obj = arg->ToObject();
  auto keys = obj->GetOwnPropertyNames();
  PyObject *dict = PyDict_New();
  for (unsigned int i = 0; i < keys->Length(); i++)
  {
    auto key = keys->Get(i);
    v8::Local<v8::Value> val = obj->Get(key);
    v8::String::Utf8Value keyStr(key);
    PyObject *pyKey = PyBytes_FromString(*keyStr);
    if (val->IsNumber())
    {
      double num = val->NumberValue();
      if (val->IsInt32())
      {
        PyDict_SetItem(dict, pyKey, PyLong_FromLong(num));
      }
      else
      {
        PyDict_SetItem(dict, pyKey, PyFloat_FromDouble(num));
      }
    }
    else if (val->IsString())
    {
      v8::String::Utf8Value str(val);
      PyDict_SetItem(dict, pyKey, PyBytes_FromString(*str));
    }
    else if (val->IsBoolean())
    {
      long b = val->BooleanValue();
      PyDict_SetItem(dict, pyKey, PyBool_FromLong(b));
    }
    else if (val->IsDate())
    {
      printf("Dates dont work yet");
      Nan::ThrowError("Dates dont work yet");

      // double millisSinceEpoch = v8::Date::Cast(*val)->NumberValue();
      // time_t t = static_cast<time_t>(millisSinceEpoch / 1000);
      // struct tm *tmp = gmtime(&t);
      // PyDateTime_FromDateAndTime(tmp->tm_year, tmp->tm_mon, tmp->tm_mday, tmp->tm_hour, tmp->tm_min, tmp->tm_sec, 0)
      // auto d = PyDateTime_FromDateAndTime(200, 6, 18, 20, 10, 10, 0);
      // printf("got the d\n");
      // PyTuple_SetItem(
      //     pArgs,
      //     i - 1,
      //     d);
      // printf("after date set\n");
    }
    else if (val->IsArray())
    {
      PyObject *innerList = BuildPyArray(val);
      PyDict_SetItem(dict, pyKey, innerList);
    }
    else if (val->IsObject())
    {
      PyObject *innerDict = BuildPyDict(val);
      PyDict_SetItem(dict, pyKey, innerDict);
    }
    else if (val->IsUint32())
    {
      // args.GetReturnValue().Set(Nan::New("uint32").ToLocalChecked());
    }
  }
  return dict;
}

PyObject *BuildPyArray(v8::Local<v8::Value> arg)
{
  v8::Local<v8::Array> arr = v8::Local<v8::Array>::Cast(arg);
  PyObject *list = PyList_New(arr->Length());

  for (unsigned int i = 0; i < arr->Length(); i++)
  {
    auto element = arr->Get(i);
    if (element->IsNumber())
    {
      double num = element->NumberValue();
      if (element->IsInt32())
      {
        PyList_SetItem(list, i, PyLong_FromLong(num));
      }
      else
      {
        PyList_SetItem(list, i, PyFloat_FromDouble(num));
      }
    }
    else if (element->IsString())
    {
      v8::String::Utf8Value str(element);
      PyList_SetItem(list, i, PyBytes_FromString(*str));
    }
    else if (element->IsBoolean())
    {
      long b = element->BooleanValue();
      PyList_SetItem(list, i, PyBool_FromLong(b));
    }
    else if (element->IsDate())
    {
      printf("Dates dont work yet");
      Nan::ThrowError("Dates dont work yet");

      // double millisSinceEpoch = v8::Date::Cast(*element)->NumberValue();
      // time_t t = static_cast<time_t>(millisSinceEpoch / 1000);
      // struct tm *tmp = gmtime(&t);
      // PyDateTime_FromDateAndTime(tmp->tm_year, tmp->tm_mon, tmp->tm_mday, tmp->tm_hour, tmp->tm_min, tmp->tm_sec, 0)
      // auto d = PyDateTime_FromDateAndTime(200, 6, 18, 20, 10, 10, 0);
      // printf("got the d\n");
      // PyTuple_SetItem(
      //     pArgs,
      //     i - 1,
      //     d);
      // printf("after date set\n");
    }
    else if (element->IsArray())
    {
      PyObject *innerList = BuildPyArray(element);
      PyList_SetItem(list, i, innerList);
    }
    else if (element->IsObject())
    {
      PyObject *innerDict = BuildPyDict(element);
      PyList_SetItem(list, i, innerDict);
    }
    else if (element->IsUint32())
    {
      // args.GetReturnValue().Set(Nan::New("uint32").ToLocalChecked());
    }
  }

  return list;
}

PyObject *BuildPyArgs(const Nan::FunctionCallbackInfo<v8::Value> &args)
{
  PyObject *pArgs = PyTuple_New(args.Length() - 1);
  for (int i = 1; i < args.Length(); i++)
  {
    auto arg = args[i];
    if (arg->IsNumber())
    {
      double num = arg->NumberValue();
      if (arg->IsInt32())
      {
        PyTuple_SetItem(pArgs, i - 1, PyLong_FromLong(num));
      }
      else
      {
        PyTuple_SetItem(pArgs, i - 1, PyFloat_FromDouble(num));
      }
    }
    else if (arg->IsString())
    {
      v8::String::Utf8Value str(arg);
      PyTuple_SetItem(pArgs, i - 1, PyUnicode_FromString(*str));
    }
    else if (arg->IsBoolean())
    {
      long b = arg->BooleanValue();
      PyTuple_SetItem(pArgs, i - 1, PyBool_FromLong(b));
    }
    else if (arg->IsDate())
    {
      printf("Dates dont work yet");
      Nan::ThrowError("Dates dont work yet");

      // double millisSinceEpoch = v8::Date::Cast(*arg)->NumberValue();
      // time_t t = static_cast<time_t>(millisSinceEpoch / 1000);
      // struct tm *tmp = gmtime(&t);
      // PyDateTime_FromDateAndTime(tmp->tm_year, tmp->tm_mon, tmp->tm_mday, tmp->tm_hour, tmp->tm_min, tmp->tm_sec, 0)
      // auto d = PyDateTime_FromDateAndTime(200, 6, 18, 20, 10, 10, 0);
      // printf("got the d\n");
      // PyTuple_SetItem(
      //     pArgs,
      //     i - 1,
      //     d);
      // printf("after date set\n");
    }
    else if (arg->IsArray())
    {
      PyObject *list = BuildPyArray(arg);
      PyTuple_SetItem(pArgs, i - 1, list);
    }
    else if (arg->IsObject())
    {
      PyObject *dict = BuildPyDict(arg);
      PyTuple_SetItem(pArgs, i - 1, dict);
    }
    else if (arg->IsUint32())
    {
      Nan::ThrowError("Uint32 not supported yet");
      // args.GetReturnValue().Set(Nan::New("uint32").ToLocalChecked());
    }
  }

  return pArgs;
}

/**
 * PyObject can be either a list or tuple
 */
v8::Local<v8::Array> BuildV8Array(PyObject *obj)
{
  Py_ssize_t len = PyList_Check(obj) ? PyList_Size(obj) : PyTuple_Size(obj);

  v8::Local<v8::Array> arr = Nan::New<v8::Array>(len);
  for (Py_ssize_t i = 0; i < len; i++)
  {
    PyObject *localObj = PyList_Check(obj) ? PyList_GetItem(obj, i) : PyTuple_GetItem(obj, i);
    if (!localObj)
      continue;
    if (PyLong_Check(localObj))
    {
      double result = PyLong_AsDouble(localObj);
      arr->Set(i, Nan::New(result));
    }
    else if (PyUnicode_Check(localObj))
    {
      auto str = Nan::New(PyUnicode_AsUTF8(localObj)).ToLocalChecked();
      arr->Set(i, str);
    }
    else if (PyFloat_Check(localObj))
    {
      double result = PyFloat_AsDouble(localObj);
      arr->Set(i, Nan::New(result));
    }
    else if (PyBytes_Check(localObj))
    {
      auto str = Nan::New(PyBytes_AsString(localObj)).ToLocalChecked();
      arr->Set(i, str);
    }
    else if (PyBool_Check(localObj))
    {
      bool b = PyObject_IsTrue(localObj);
      arr->Set(i, Nan::New(b));
    }
    else if (PyList_Check(localObj) || PyTuple_Check(localObj))
    {
      auto innerArr = BuildV8Array(localObj);
      arr->Set(i, innerArr);
    }
    else if (PyDict_Check(localObj))
    {
      auto innerDict = BuildV8Dict(localObj);
      arr->Set(i, innerDict);
    }
  }
  return arr;
}

char * getKey(PyObject *key) {
  if (PyUnicode_Check(key)) {
    return PyUnicode_AsUTF8(key);
  } else {
    return PyBytes_AsString(key);
  }
}

v8::Local<v8::Object> BuildV8Dict(PyObject *obj)
{
  auto keys = PyDict_Keys(obj);
  auto size = PyList_GET_SIZE(keys);
  v8::Local<v8::Object> jsObj = Nan::New<v8::Object>();
  for (Py_ssize_t i = 0; i < size; i++)
  {
    auto key = PyList_GetItem(keys, i);
    auto val = PyDict_GetItem(obj, key);
    auto jsKey = Nan::New(getKey(key)).ToLocalChecked();
    if (PyLong_Check(val))
    {
      double result = PyLong_AsDouble(val);
      jsObj->Set(jsKey, Nan::New(result));
    }
    else if (PyUnicode_Check(val))
    {
      auto str = Nan::New(PyUnicode_AsUTF8(val)).ToLocalChecked();
      jsObj->Set(jsKey, str);
    }
    else if (PyFloat_Check(val))
    {
      double result = PyFloat_AsDouble(val);
      jsObj->Set(jsKey, Nan::New(result));
    }
    else if (PyBytes_Check(val))
    {
      auto str = Nan::New(PyBytes_AsString(val)).ToLocalChecked();
      jsObj->Set(jsKey, str);
    }
    else if (PyBool_Check(val))
    {
      bool b = PyObject_IsTrue(val);
      jsObj->Set(jsKey, Nan::New(b));
    }
    else if (PyList_Check(val) || PyTuple_Check(val))
    {
      auto innerArr = BuildV8Array(val);
      jsObj->Set(jsKey, innerArr);
    }
    else if (PyDict_Check(val))
    {
      auto innerDict = BuildV8Dict(val);
      jsObj->Set(jsKey, innerDict);
    }
  }
  return jsObj;
}

int Py_GetNumArguments(PyObject *pFunc)
{
  PyObject *fc = PyObject_GetAttrString(pFunc, "__code__");
  if (fc)
  {
    PyObject *ac = PyObject_GetAttrString(fc, "co_argcount");
    if (ac)
    {
      long count = PyLong_AsLong(ac);
      Py_DECREF(ac);
      return count;
    }
    Py_DECREF(fc);
  }
  return 0;
}

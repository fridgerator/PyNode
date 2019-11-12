#include "helpers.h"
#define UNUSED(expr) (void)(expr)

PyObject *BuildPyDict(v8::Local<v8::Value> arg) {
  auto obj = arg.As<v8::Object>();
  auto keys =
      obj->GetOwnPropertyNames(Nan::GetCurrentContext()).ToLocalChecked();
  PyObject *dict = PyDict_New();
  for (unsigned int i = 0; i < keys->Length(); i++) {
    auto key = keys->Get(Nan::GetCurrentContext(), i).ToLocalChecked();
    v8::Local<v8::Value> val =
        obj->Get(Nan::GetCurrentContext(), key).ToLocalChecked();
    Nan::Utf8String keyStr(key);
    PyObject *pyKey = PyBytes_FromString(*keyStr);
    if (val->IsNumber()) {
      double num = val->NumberValue(Nan::GetCurrentContext()).FromJust();
      if (val->IsInt32()) {
        PyDict_SetItem(dict, pyKey, PyLong_FromLong(num));
      } else {
        PyDict_SetItem(dict, pyKey, PyFloat_FromDouble(num));
      }
    } else if (val->IsString()) {
      Nan::Utf8String str(val);
      PyDict_SetItem(dict, pyKey, PyBytes_FromString(*str));
    } else if (val->IsBoolean()) {
#if NODE_MODULE_VERSION < NODE_12_0_MODULE_VERSION
      long b = val->BooleanValue(Nan::GetCurrentContext()).FromJust();
#else
      long b = val->BooleanValue(Nan::GetCurrentContext()->GetIsolate());
#endif
      PyDict_SetItem(dict, pyKey, PyBool_FromLong(b));
    } else if (val->IsDate()) {
      printf("Dates dont work yet");
      Nan::ThrowError("Dates dont work yet");

      // double millisSinceEpoch = v8::Date::Cast(*val)->NumberValue();
      // time_t t = static_cast<time_t>(millisSinceEpoch / 1000);
      // struct tm *tmp = gmtime(&t);
      // PyDateTime_FromDateAndTime(tmp->tm_year, tmp->tm_mon, tmp->tm_mday,
      // tmp->tm_hour, tmp->tm_min, tmp->tm_sec, 0) auto d =
      // PyDateTime_FromDateAndTime(200, 6, 18, 20, 10, 10, 0); printf("got the
      // d\n"); PyTuple_SetItem(
      //     pArgs,
      //     i - 1,
      //     d);
      // printf("after date set\n");
    } else if (val->IsArray()) {
      PyObject *innerList = BuildPyArray(val);
      PyDict_SetItem(dict, pyKey, innerList);
    } else if (val->IsObject()) {
      PyObject *innerDict = BuildPyDict(val);
      PyDict_SetItem(dict, pyKey, innerDict);
    } else if (val->IsUint32()) {
      // args.GetReturnValue().Set(Nan::New("uint32").ToLocalChecked());
    }
  }
  return dict;
}

PyObject *BuildPyArray(v8::Local<v8::Value> arg) {
  v8::Local<v8::Array> arr = v8::Local<v8::Array>::Cast(arg);
  PyObject *list = PyList_New(arr->Length());

  for (unsigned int i = 0; i < arr->Length(); i++) {
    auto element = arr->Get(Nan::GetCurrentContext(), i).ToLocalChecked();
    if (element->IsNumber()) {
      double num = element->NumberValue(Nan::GetCurrentContext()).FromJust();
      if (element->IsInt32()) {
        PyList_SetItem(list, i, PyLong_FromLong(num));
      } else {
        PyList_SetItem(list, i, PyFloat_FromDouble(num));
      }
    } else if (element->IsString()) {
      Nan::Utf8String str(element);
      PyList_SetItem(list, i, PyBytes_FromString(*str));
    } else if (element->IsBoolean()) {
#if NODE_MODULE_VERSION < NODE_12_0_MODULE_VERSION
      bool b = element->BooleanValue(Nan::GetCurrentContext()).FromJust();
#else
      bool b = element->BooleanValue(Nan::GetCurrentContext()->GetIsolate());
#endif
      PyList_SetItem(list, i, PyBool_FromLong(b));
    } else if (element->IsDate()) {
      printf("Dates dont work yet");
      Nan::ThrowError("Dates dont work yet");

      // double millisSinceEpoch = v8::Date::Cast(*element)->NumberValue();
      // time_t t = static_cast<time_t>(millisSinceEpoch / 1000);
      // struct tm *tmp = gmtime(&t);
      // PyDateTime_FromDateAndTime(tmp->tm_year, tmp->tm_mon, tmp->tm_mday,
      // tmp->tm_hour, tmp->tm_min, tmp->tm_sec, 0) auto d =
      // PyDateTime_FromDateAndTime(200, 6, 18, 20, 10, 10, 0); printf("got the
      // d\n"); PyTuple_SetItem(
      //     pArgs,
      //     i - 1,
      //     d);
      // printf("after date set\n");
    } else if (element->IsArray()) {
      PyObject *innerList = BuildPyArray(element);
      PyList_SetItem(list, i, innerList);
    } else if (element->IsObject()) {
      PyObject *innerDict = BuildPyDict(element);
      PyList_SetItem(list, i, innerDict);
    } else if (element->IsUint32()) {
      // args.GetReturnValue().Set(Nan::New("uint32").ToLocalChecked());
    }
  }

  return list;
}

PyObject *BuildPyArgs(const v8::FunctionCallbackInfo<v8::Value> &args) {
  // Arguments length minus 2: one for function name, one for js callback
  PyObject *pArgs = PyTuple_New(args.Length() - 2);
  for (int i = 1; i < args.Length() - 1; i++) {
    auto arg = args[i];
    if (arg->IsNumber()) {
      double num = arg->NumberValue(Nan::GetCurrentContext()).FromJust();
      if (arg->IsInt32()) {
        PyTuple_SetItem(pArgs, i - 1, PyLong_FromLong(num));
      } else {
        PyTuple_SetItem(pArgs, i - 1, PyFloat_FromDouble(num));
      }
    } else if (arg->IsString()) {
      Nan::Utf8String str(arg);
      PyTuple_SetItem(pArgs, i - 1, PyUnicode_FromString(*str));
    } else if (arg->IsBoolean()) {
#if NODE_MODULE_VERSION < NODE_12_0_MODULE_VERSION
      long b = arg->BooleanValue(Nan::GetCurrentContext()).FromJust();
#else
      long b = arg->BooleanValue(Nan::GetCurrentContext()->GetIsolate());
#endif
      PyTuple_SetItem(pArgs, i - 1, PyBool_FromLong(b));
    } else if (arg->IsDate()) {
      printf("Dates dont work yet");
      Nan::ThrowError("Dates dont work yet");

      // double millisSinceEpoch = v8::Date::Cast(*arg)->NumberValue();
      // time_t t = static_cast<time_t>(millisSinceEpoch / 1000);
      // struct tm *tmp = gmtime(&t);
      // PyDateTime_FromDateAndTime(tmp->tm_year, tmp->tm_mon, tmp->tm_mday,
      // tmp->tm_hour, tmp->tm_min, tmp->tm_sec, 0) auto d =
      // PyDateTime_FromDateAndTime(200, 6, 18, 20, 10, 10, 0); printf("got the
      // d\n"); PyTuple_SetItem(
      //     pArgs,
      //     i - 1,
      //     d);
      // printf("after date set\n");
    } else if (arg->IsArray()) {
      PyObject *list = BuildPyArray(arg);
      PyTuple_SetItem(pArgs, i - 1, list);
    } else if (arg->IsObject()) {
      PyObject *dict = BuildPyDict(arg);
      PyTuple_SetItem(pArgs, i - 1, dict);
    } else if (arg->IsUint32()) {
      Nan::ThrowError("Uint32 not supported yet");
      // args.GetReturnValue().Set(Nan::New("uint32").ToLocalChecked());
    }
  }

  return pArgs;
}

/**
 * PyObject can be either a list or tuple
 */
v8::Local<v8::Array> BuildV8Array(PyObject *obj) {
  Py_ssize_t len = PyList_Size(obj);

  v8::Local<v8::Array> arr = Nan::New<v8::Array>(len);
  for (Py_ssize_t i = 0; i < len; i++) {
    PyObject *localObj;
    if (strcmp(obj->ob_type->tp_name, "list") == 0) {
      localObj = PyList_GetItem(obj, i);
    } else {
      localObj = PyTuple_GetItem(obj, i);
    }
    if (!localObj)
      continue;
    if (strcmp(localObj->ob_type->tp_name, "int") == 0) {
      double result = PyLong_AsDouble(localObj);
      auto unused_result =
          arr->Set(Nan::GetCurrentContext(), i, Nan::New(result));
      UNUSED(unused_result);
    } else if (strcmp(localObj->ob_type->tp_name, "str") == 0) {
      auto str = Nan::New(PyUnicode_AsUTF8(localObj)).ToLocalChecked();
      auto unused_result = arr->Set(Nan::GetCurrentContext(), i, str);
      UNUSED(unused_result);
    } else if (strcmp(localObj->ob_type->tp_name, "float") == 0) {
      double result = PyFloat_AsDouble(localObj);
      auto unused_result =
          arr->Set(Nan::GetCurrentContext(), i, Nan::New(result));
      UNUSED(unused_result);
    } else if (strcmp(localObj->ob_type->tp_name, "bytes") == 0) {
      auto str = Nan::New(PyBytes_AsString(localObj)).ToLocalChecked();
      auto unused_result = arr->Set(Nan::GetCurrentContext(), i, str);
      UNUSED(unused_result);
    } else if (strcmp(localObj->ob_type->tp_name, "bool") == 0) {
      bool b = PyObject_IsTrue(localObj);
      auto unused_result = arr->Set(Nan::GetCurrentContext(), i, Nan::New(b));
      UNUSED(unused_result);
    } else if (strcmp(localObj->ob_type->tp_name, "list") == 0) {
      auto innerArr = BuildV8Array(localObj);
      auto unused_result = arr->Set(Nan::GetCurrentContext(), i, innerArr);
      UNUSED(unused_result);
    } else if (strcmp(localObj->ob_type->tp_name, "dict") == 0) {
      auto innerDict = BuildV8Dict(localObj);
      auto unused_result = arr->Set(Nan::GetCurrentContext(), i, innerDict);
      UNUSED(unused_result);
    }
  }
  return arr;
}

std::string getKey(PyObject *key) {
  if (strcmp(key->ob_type->tp_name, "str") == 0) {
    return std::string(PyUnicode_AsUTF8(key));
  } else {
    return std::string(PyBytes_AsString(key));
  }
}

v8::Local<v8::Object> BuildV8Dict(PyObject *obj) {
  auto keys = PyDict_Keys(obj);
  auto size = PyList_GET_SIZE(keys);
  v8::Local<v8::Object> jsObj = Nan::New<v8::Object>();
  for (Py_ssize_t i = 0; i < size; i++) {
    auto key = PyList_GetItem(keys, i);
    auto val = PyDict_GetItem(obj, key);
    auto jsKey = Nan::New(getKey(key)).ToLocalChecked();
    if (strcmp(val->ob_type->tp_name, "int") == 0) {
      double result = PyLong_AsDouble(val);
      auto unused_result =
          jsObj->Set(Nan::GetCurrentContext(), jsKey, Nan::New(result));
      UNUSED(unused_result);
    } else if (strcmp(val->ob_type->tp_name, "str") == 0) {
      auto str = Nan::New(PyUnicode_AsUTF8(val)).ToLocalChecked();
      auto unused_result = jsObj->Set(Nan::GetCurrentContext(), jsKey, str);
      UNUSED(unused_result);
    } else if (strcmp(val->ob_type->tp_name, "float") == 0) {
      double result = PyFloat_AsDouble(val);
      auto unused_result =
          jsObj->Set(Nan::GetCurrentContext(), jsKey, Nan::New(result));
      UNUSED(unused_result);
    } else if (strcmp(val->ob_type->tp_name, "bytes") == 0) {
      auto str = Nan::New(PyBytes_AsString(val)).ToLocalChecked();
      auto unused_result = jsObj->Set(Nan::GetCurrentContext(), jsKey, str);
      UNUSED(unused_result);
    } else if (strcmp(val->ob_type->tp_name, "bool") == 0) {
      bool b = PyObject_IsTrue(val);
      auto unused_result =
          jsObj->Set(Nan::GetCurrentContext(), jsKey, Nan::New(b));
      UNUSED(unused_result);
    } else if (strcmp(val->ob_type->tp_name, "list") == 0) {
      auto innerArr = BuildV8Array(val);
      auto unused_result =
          jsObj->Set(Nan::GetCurrentContext(), jsKey, innerArr);
      UNUSED(unused_result);
    } else if (strcmp(val->ob_type->tp_name, "dict") == 0) {
      auto innerDict = BuildV8Dict(val);
      auto unused_result =
          jsObj->Set(Nan::GetCurrentContext(), jsKey, innerDict);
      UNUSED(unused_result);
    }
  }
  return jsObj;
}

int Py_GetNumArguments(PyObject *pFunc) {
  PyObject *fc = PyObject_GetAttrString(pFunc, "__code__");
  if (fc) {
    PyObject *ac = PyObject_GetAttrString(fc, "co_argcount");
    if (ac) {
      long count = PyLong_AsLong(ac);
      Py_DECREF(ac);
      return count;
    }
    Py_DECREF(fc);
  }
  return 0;
}

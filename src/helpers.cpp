#include "helpers.hpp"
#include <iostream>

bool isNapiValueInt(Napi::Env &env, Napi::Value &num) {
  return env.Global()
      .Get("Number")
      .ToObject()
      .Get("isInteger")
      .As<Napi::Function>()
      .Call({num})
      .ToBoolean()
      .Value();
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

PyObject *BuildPyArray(Napi::Env env, Napi::Value arg) {
  auto arr = arg.As<Napi::Array>();
  PyObject *list = PyList_New(arr.Length());

  for (size_t i = 0; i < arr.Length(); i++) {
    auto element = arr.Get(i);
    if (element.IsNumber()) {
      double num = element.ToNumber();
      if (isNapiValueInt(env, element)) {
        PyList_SetItem(list, i, PyLong_FromLong(num));
      } else {
        PyList_SetItem(list, i, PyFloat_FromDouble(num));
      }
    } else if (element.IsString()) {
      std::string str = element.As<Napi::String>().ToString();
      PyList_SetItem(list, i, PyBytes_FromString(str.c_str()));
    } else if (element.IsBoolean()) {
      bool b = element.ToBoolean();
      PyList_SetItem(list, i, PyBool_FromLong(b));
    } else if (element.IsArray()) {
      PyObject *innerList = BuildPyArray(env, element);
      PyList_SetItem(list, i, innerList);
    } else if (element.IsObject()) {
      // build py dict
      PyObject *innerDict = BuildPyDict(env, element);
      PyList_SetItem(list, i, innerDict);
    }
  }

  return list;
}

PyObject *BuildPyDict(Napi::Env env, Napi::Value arg) {
  auto obj = arg.As<Napi::Object>();
  auto keys = obj.GetPropertyNames();
  PyObject *dict = PyDict_New();
  for (size_t i = 0; i < keys.Length(); i++) {
    auto key = keys.Get(i);
    std::string keyStr = key.ToString();
    Napi::Value val = obj.Get(key);
    PyObject *pykey = PyBytes_FromString(keyStr.c_str());
    if (val.IsNumber()) {
      double num = val.ToNumber();
      if (isNapiValueInt(env, val)) {
        PyDict_SetItem(dict, pykey, PyLong_FromLong(num));
      } else {
        PyDict_SetItem(dict, pykey, PyFloat_FromDouble(num));
      }
    } else if (val.IsString()) {
      std::string str = val.ToString();
      PyDict_SetItem(dict, pykey, PyBytes_FromString(str.c_str()));
    } else if (val.IsArray()) {
      PyObject *innerList = BuildPyArray(env, val);
      PyDict_SetItem(dict, pykey, innerList);
    } else if (val.IsObject()) {
      PyObject *innerDict = BuildPyDict(env, val);
      PyDict_SetItem(dict, pykey, innerDict);
    }
  }

  return dict;
}

PyObject *BuildPyArgs(const Napi::CallbackInfo &args) {
  Napi::Env env = args.Env();
  // Arguments length minus 2: one for function name, one for js callback
  PyObject *pArgs = PyTuple_New(args.Length() - 2);
  for (size_t i = 1; i < args.Length() - 1; i++) {
    auto arg = args[i];
    if (arg.IsNumber()) {
      double num = arg.As<Napi::Number>().ToNumber();
      if (isNapiValueInt(env, arg)) {
        PyTuple_SetItem(pArgs, i - 1, PyLong_FromLong(num));
      } else {
        PyTuple_SetItem(pArgs, i - 1, PyFloat_FromDouble(num));
      }
    } else if (arg.IsString()) {
      std::string str = arg.As<Napi::String>().ToString();
      PyTuple_SetItem(pArgs, i - 1, PyUnicode_FromString(str.c_str()));
    } else if (arg.IsBoolean()) {
      long b = arg.As<Napi::Boolean>().ToBoolean();
      PyTuple_SetItem(pArgs, i - 1, PyBool_FromLong(b));
      // } else if (arg.IsDate()) {
      //   printf("Dates dont work yet");
      //   // Nan::ThrowError("Dates dont work yet");
      //   throw Napi::Error::New(args.Env(), "Dates dont work  yet");
    } else if (arg.IsArray()) {
      PyObject *list = BuildPyArray(env, arg);
      PyTuple_SetItem(pArgs, i - 1, list);
    } else if (arg.IsObject()) {
      PyObject *dict = BuildPyDict(env, arg);
      PyTuple_SetItem(pArgs, i - 1, dict);
    }
  }

  return pArgs;
}

Napi::Array BuildV8Array(Napi::Env env, PyObject *obj) {
  Py_ssize_t len = PyList_Size(obj);

  auto arr = Napi::Array::New(env);

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
      arr.Set(i, result);
    } else if (strcmp(localObj->ob_type->tp_name, "str") == 0) {
      auto str = PyUnicode_AsUTF8(localObj);
      arr.Set(i, str);
    } else if (strcmp(localObj->ob_type->tp_name, "float") == 0) {
      double result = PyFloat_AsDouble(localObj);
      arr.Set(i, result);
    } else if (strcmp(localObj->ob_type->tp_name, "bytes") == 0) {
      auto str = PyBytes_AsString(localObj);
      arr.Set(i, str);
    } else if (strcmp(localObj->ob_type->tp_name, "bool") == 0) {
      bool b = PyObject_IsTrue(localObj);
      arr.Set(i, b);
    } else if (strcmp(localObj->ob_type->tp_name, "list") == 0) {
      auto innerArr = BuildV8Array(env, localObj);
      arr.Set(i, innerArr);
    } else if (strcmp(localObj->ob_type->tp_name, "dict") == 0) {
      auto innerDict = BuildV8Dict(env, localObj);
      arr.Set(i, innerDict);
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

Napi::Object BuildV8Dict(Napi::Env env, PyObject *obj) {
  auto keys = PyDict_Keys(obj);
  auto size = PyList_GET_SIZE(keys);
  auto jsObj = Napi::Object::New(env);

  for (Py_ssize_t i = 0; i < size; i++) {
    auto key = PyList_GetItem(keys, i);
    auto val = PyDict_GetItem(obj, key);
    auto jsKey = Napi::String::New(env, getKey(key));
    std::string kk = jsKey.As<Napi::String>().ToString();

    if (strcmp(val->ob_type->tp_name, "int") == 0) {
      double result = PyLong_AsDouble(val);
      jsObj.Set(jsKey, result);
    } else if (strcmp(val->ob_type->tp_name, "str") == 0) {
      auto str = PyUnicode_AsUTF8(val);
      jsObj.Set(jsKey, str);
    } else if (strcmp(val->ob_type->tp_name, "float") == 0) {
      double result = PyFloat_AsDouble(val);
      jsObj.Set(jsKey, result);
    } else if (strcmp(val->ob_type->tp_name, "bytes") == 0) {
      auto str = PyBytes_AsString(val);
      jsObj.Set(jsKey, str);
    } else if (strcmp(val->ob_type->tp_name, "bool") == 0) {
      bool b = PyObject_IsTrue(val);
      jsObj.Set(jsKey, b);
    } else if (strcmp(val->ob_type->tp_name, "list") == 0) {
      auto innerArr = BuildV8Array(env, val);
      jsObj.Set(jsKey, innerArr);
    } else if (strcmp(val->ob_type->tp_name, "dict") == 0) {
      auto innerDict = BuildV8Dict(env, val);
      jsObj.Set(jsKey, innerDict);
    }
  }

  return jsObj;
}

#include "helpers.hpp"
#include "jswrapper.h"
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

/* Returns true if the value given is (roughly) an object literal,
 * ie more appropriate as a Python dict than a WrappedJSObject.
 *
 * Based on https://stackoverflow.com/questions/5876332/how-can-i-differentiate-between-an-object-literal-other-javascript-objects
 */
bool isNapiValuePlainObject(Napi::Env &env, Napi::Value &obj) {
    napi_value result;
    napi_value plainobj_result;
    napi_status status;

    status = napi_get_prototype(env, obj, &result);
    if (status != napi_ok) {
        return false;
    }

    Napi::Object plainobj = Napi::Object::New(env);
    status = napi_get_prototype(env, plainobj, &plainobj_result);
    if (status != napi_ok) {
        return false;
    }

    bool equal;
    status = napi_strict_equals(env, result, plainobj_result, &equal);
    return equal;
}

bool isNapiValueWrappedPython(Napi::Env &env, Napi::Object obj) {
    return obj.InstanceOf(PyNodeWrappedPythonObject::constructor.Value());
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
    PyObject * pyval = ConvertToPython(element);
    if (pyval != NULL) {
      PyList_SetItem(list, i, pyval);
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
    PyObject *pykey = PyUnicode_FromString(keyStr.c_str());
    PyObject *pyval = ConvertToPython(val);
    if (pyval != NULL) {
      PyDict_SetItem(dict, pykey, pyval);
    }
  }

  return dict;
}

PyObject *BuildWrappedJSObject(Napi::Env env, Napi::Value arg) {
  PyObject *pyobj = WrappedJSObject_New(env, arg);
  return pyobj;
}

PyObject *BuildPyArgs(const Napi::CallbackInfo &args, size_t start_index, size_t count) {
  PyObject *pArgs = PyTuple_New(count);
  for (size_t i = start_index; i < start_index + count; i++) {
    auto arg = args[i];
    PyObject *pyobj = ConvertToPython(arg);
    if (pyobj != NULL) {
      PyTuple_SetItem(pArgs, i - start_index, pyobj);
    }
  }

  return pArgs;
}

PyObject * ConvertToPython(Napi::Value arg) {
    Napi::Env env = arg.Env();
    if (arg.IsNumber()) {
      double num = arg.As<Napi::Number>().ToNumber();
      if (isNapiValueInt(env, arg)) {
        return PyLong_FromLong(num);
      } else {
        return PyFloat_FromDouble(num);
      }
    } else if (arg.IsString()) {
      std::string str = arg.As<Napi::String>().ToString();
      return PyUnicode_FromString(str.c_str());
    } else if (arg.IsBoolean()) {
      long b = arg.As<Napi::Boolean>().ToBoolean();
      return PyBool_FromLong(b);
      // } else if (arg.IsDate()) {
      //   printf("Dates dont work yet");
      //   // Nan::ThrowError("Dates dont work yet");
      //   throw Napi::Error::New(args.Env(), "Dates dont work  yet");
    } else if (arg.IsArray()) {
      return BuildPyArray(env, arg);
    } else if (arg.IsObject()) {
      if (isNapiValueWrappedPython(env, arg.ToObject())) {
        Napi::Object o = arg.ToObject();
        PyNodeWrappedPythonObject *wrapper = Napi::ObjectWrap<PyNodeWrappedPythonObject>::Unwrap(o);
        PyObject* pyobj = wrapper->getValue();
        return pyobj;
      } else if (isNapiValuePlainObject(env, arg)) {
        return BuildPyDict(env, arg);
      } else {
        return BuildWrappedJSObject(env, arg);
      }
    } else if (arg.IsNull() || arg.IsUndefined()) {
      Py_RETURN_NONE;
    } else {
      Napi::String string = arg.ToString();
      std::cout << "Unknown arg type" << string.Utf8Value() << std::endl;
      throw Napi::Error::New(arg.Env(), "Unknown arg type");
    }
}

extern "C" {
    PyObject * convert_napi_value_to_python(napi_env env, napi_value value) {
        Napi::Value cpp_value = Napi::Value(env, value);
        return ConvertToPython(cpp_value);
    }
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

Napi::Value ConvertFromPython(Napi::Env env, PyObject * pValue) {
    Napi::Value result = env.Null();
    if (strcmp(pValue->ob_type->tp_name, "NoneType") == 0) {
      // leave as null
    } else if (strcmp(pValue->ob_type->tp_name, "bool") == 0) {
      bool b = PyObject_IsTrue(pValue);
      result = Napi::Boolean::New(env, b);
    } else if (strcmp(pValue->ob_type->tp_name, "int") == 0) {
      double d = PyLong_AsDouble(pValue);
      result = Napi::Number::New(env, d);
    } else if (strcmp(pValue->ob_type->tp_name, "float") == 0) {
      double d = PyFloat_AsDouble(pValue);
      result = Napi::Number::New(env, d);
    } else if (strcmp(pValue->ob_type->tp_name, "bytes") == 0) {
      auto str = Napi::String::New(env, PyBytes_AsString(pValue));
      result = str;
    } else if (strcmp(pValue->ob_type->tp_name, "str") == 0) {
      auto str = Napi::String::New(env, PyUnicode_AsUTF8(pValue));
      result = str;
    } else if (strcmp(pValue->ob_type->tp_name, "list") == 0) {
      auto arr = BuildV8Array(env, pValue);
      result = arr;
    } else if (strcmp(pValue->ob_type->tp_name, "dict") == 0) {
      auto obj = BuildV8Dict(env, pValue);
      result = obj;
    } else if (strcmp(pValue->ob_type->tp_name, "pynode.WrappedJSObject") == 0) {
      auto obj = Napi::Value(env, WrappedJSObject_get_napi_value(pValue));
      result = obj;
    } else {
      auto exp = Napi::External<PyObject>::New(env, pValue);
      auto obj = PyNodeWrappedPythonObject::constructor.New({exp});
      result = obj;
    }
    return result;
}

extern "C" {
    napi_value convert_python_to_napi_value(napi_env env, PyObject * obj) {
        return ConvertFromPython(env, obj);
    }
}


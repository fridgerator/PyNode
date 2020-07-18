#include "pywrapper.hpp"
#include <napi.h>
#include <iostream>

Napi::Object PyNodeWrappedPythonObject::Init(Napi::Env env, Napi::Object exports) {
    // This method is used to hook the accessor and method callbacks
    Napi::Function func = DefineClass(env, "PyNodeWrappedPythonObject", {
        InstanceMethod("call", &PyNodeWrappedPythonObject::Call),
        InstanceMethod("get", &PyNodeWrappedPythonObject::GetAttr),
        InstanceMethod("repr", &PyNodeWrappedPythonObject::Repr)
    });

    // Create a peristent reference to the class constructor. This will allow
    // a function called on a class prototype and a function
    // called on instance of a class to be distinguished from each other.
    constructor = Napi::Persistent(func);
    // Call the SuppressDestruct() method on the static data prevent the calling
    // to this destructor to reset the reference when the environment is no longer
    // available.
    constructor.SuppressDestruct();
    exports.Set("PyNodeWrappedPythonObject", func);
    return exports;
}

PyNodeWrappedPythonObject::PyNodeWrappedPythonObject(const Napi::CallbackInfo &info) : Napi::ObjectWrap<PyNodeWrappedPythonObject>(info) {
    //Napi::Env env = info.Env();
    // ...
    PyObject * value = info[0].As<Napi::External<PyObject>>().Data();
    this->_value = value;
    Py_INCREF(this->_value);
    // TODO - Py_DECREF in destructor
}

Napi::FunctionReference PyNodeWrappedPythonObject::constructor;

Napi::Value PyNodeWrappedPythonObject::GetAttr(const Napi::CallbackInfo &info){
    py_ensure_gil ctx;
    Napi::Env env = info.Env();
    std::string attrname = info[0].As<Napi::String>().ToString();
    PyObject * attr = PyObject_GetAttrString(this->_value, attrname.c_str());
    if (attr == NULL) {
        std::string error("Attribute " + attrname + " not found.");
        Napi::Error::New(env, error).ThrowAsJavaScriptException();
        return env.Null();
    }
    Napi::Value returnval = ConvertFromPython(env, attr);
    return returnval;
}

Napi::Value PyNodeWrappedPythonObject::Call(const Napi::CallbackInfo &info){
    py_ensure_gil ctx;
    Napi::Env env = info.Env();
    int callable = PyCallable_Check(this->_value);
    if (! callable) {
        std::string error("This Python object is not callable.");
        Napi::Error::New(env, error).ThrowAsJavaScriptException();
        return env.Null();
    }
    PyObject * pArgs = BuildPyArgs(info, 0, info.Length());
    PyObject * pReturnValue = PyObject_Call(this->_value, pArgs, NULL);
    Py_DECREF(pArgs);
    PyObject *error_occurred = PyErr_Occurred();
    if (error_occurred != NULL) {
        // TODO - get the traceback string into Javascript
        std::string error("A Python error occurred.");
        PyErr_Print();
        Napi::Error::New(env, error).ThrowAsJavaScriptException();
        return env.Null();
    }
    Napi::Value returnval = ConvertFromPython(env, pReturnValue);
    Py_DECREF(pReturnValue);
    return returnval;
}

Napi::Value PyNodeWrappedPythonObject::Repr(const Napi::CallbackInfo &info){
    py_ensure_gil ctx;
    Napi::Env env = info.Env();
    std::string attrname = info[0].As<Napi::String>().ToString();
    PyObject * repr = PyObject_Repr(this->_value);
    if (repr == NULL) {
        std::string error("repr() failed.");
        Napi::Error::New(env, error).ThrowAsJavaScriptException();
        return env.Null();
    }
    const char * repr_c = PyUnicode_AsUTF8(repr);
    Napi::Value result = Napi::String::New(env, repr_c); // (Napi takes ownership of repr_c)
    return result;
}


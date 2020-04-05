#include "pywrapper.hpp"
#include <napi.h>

Napi::Object PyNodeWrappedPythonObject::Init(Napi::Env env, Napi::Object exports) {
    // This method is used to hook the accessor and method callbacks
    Napi::Function func = DefineClass(env, "PyNodeWrappedPythonObject", {
        InstanceMethod("GetValue", &PyNodeWrappedPythonObject::GetValue),
        InstanceMethod("SetValue", &PyNodeWrappedPythonObject::SetValue)
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
}

Napi::FunctionReference PyNodeWrappedPythonObject::constructor;

Napi::Value PyNodeWrappedPythonObject::GetValue(const Napi::CallbackInfo &info){
    Napi::Env env = info.Env();
    return Napi::Number::New(env, 0.0);
}

Napi::Value PyNodeWrappedPythonObject::SetValue(const Napi::CallbackInfo &info){
    //Napi::Env env = info.Env();
    // ...
    PyObject * value = info[0].As<Napi::External<PyObject>>().Data();
    this->_value = value;
    return this->GetValue(info);
}


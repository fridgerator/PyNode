#include "napi.h"
#include "pynode.hpp"
#include <iostream>

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  exports = PyNodeInit(env, exports);

  return exports;
}

NODE_API_MODULE(PyNode, Init)

#ifndef PYNODE_HELPERS_H
#define PYNODE_HELPERS_H

#include <node_api.h>

#ifdef __cplusplus
extern "C" {
#endif

PyObject * convert_napi_value_to_python(napi_env, napi_value);

#ifdef __cplusplus
}
#endif

#endif

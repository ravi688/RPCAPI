
#pragma once
#include <RPCAPI/api_defines.hpp>
#if !defined(RPCAPI_RELEASE) && !defined(RPCAPI_DEBUG)
#   warning "None of RPCAPI_RELEASE && RPCAPI_DEBUG is defined; using RPCAPI_DEBUG"
#   define RPCAPI_DEBUG
#endif



#pragma once

#if (defined _WIN32 || defined __CYGWIN__) && defined(__GNUC__)
#	define RPCAPI_IMPORT_API __declspec(dllimport)
#	define RPCAPI_EXPORT_API __declspec(dllexport)
#else
#	define RPCAPI_IMPORT_API __attribute__((visibility("default")))
#	define RPCAPI_EXPORT_API __attribute__((visibility("default")))
#endif

#ifdef RPCAPI_BUILD_STATIC_LIBRARY
#	define RPCAPI_API
#elif defined(RPCAPI_BUILD_DYNAMIC_LIBRARY)
#	define RPCAPI_API RPCAPI_EXPORT_API
#elif defined(RPCAPI_USE_DYNAMIC_LIBRARY)
#	define RPCAPI_API RPCAPI_IMPORT_API
#elif defined(RPCAPI_USE_STATIC_LIBRARY)
#	define RPCAPI_API
#else
#	define RPCAPI_API
#endif


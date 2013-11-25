// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#if defined (_WINDOWS)
#if defined(WEBSOCKET_MODULE_EXPORTS)
#define WEBSOCKET_SERVER_MODULE_API __declspec(dllexport)
#else
#define WEBSOCKET_SERVER_MODULE_API __declspec(dllimport)
#endif
#else
#define WEBSOCKET_SERVER_MODULE_API
#endif

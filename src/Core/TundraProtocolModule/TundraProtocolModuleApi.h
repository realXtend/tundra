// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#if defined (_WINDOWS)
#if defined(TUNDRAPROTOCOL_MODULE_EXPORTS)
#define TUNDRAPROTOCOL_MODULE_API __declspec(dllexport)
#else
#define TUNDRAPROTOCOL_MODULE_API __declspec(dllimport)
#endif
#else
#define TUNDRAPROTOCOL_MODULE_API
#endif

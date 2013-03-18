// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#if defined(_WINDOWS)
#if defined(SKYX_HYDRAX_EXPORTS)
#define SKYX_HYDRAX_API __declspec(dllexport)
#else
#define SKYX_HYDRAX_API __declspec(dllimport)
#endif
#else
#define SKYX_HYDRAX_API
#endif

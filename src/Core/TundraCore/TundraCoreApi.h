// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#if defined(_WINDOWS) && defined(TUNDRACORE_SHARED)
#if defined(TUNDRACORE_EXPORTS)
#define TUNDRACORE_API __declspec(dllexport)
#else
#define TUNDRACORE_API __declspec(dllimport)
#endif
#else
#define TUNDRACORE_API
#endif

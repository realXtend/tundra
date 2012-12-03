// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#if defined (_WINDOWS)
#if defined(OPENASSETIMPORT_EXPORTS) 
#define OPENASSETIMPORT_API __declspec(dllexport)
#else
#define OPENASSETIMPORT_API __declspec(dllimport) 
#endif
#else
#define OPENASSETIMPORT_API
#endif


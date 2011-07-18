// For conditions of distribution and use, see copyright notice in license.txt

#pragma once

#if defined (_WINDOWS)
#if defined(DEBUGSTATS_MODULE_EXPORTS) 
#define DEBUGSTATS_MODULE_API __declspec(dllexport)
#else
#define DEBUGSTATS_MODULE_API __declspec(dllimport) 
#endif
#else
#define DEBUGSTATS_MODULE_API
#endif


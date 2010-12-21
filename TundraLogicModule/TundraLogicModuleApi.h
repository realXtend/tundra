// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_TundraLogiclModuleApi_h
#define incl_TundraLogiclModuleApi_h

#if defined (_WINDOWS)
#if defined(TUNDRALOGIC_MODULE_EXPORTS) 
#define TUNDRALOGIC_MODULE_API __declspec(dllexport)
#else
#define TUNDRALOGIC_MODULE_API __declspec(dllimport) 
#endif
#else
#define TUNDRALOGIC_MODULE_API
#endif

#endif

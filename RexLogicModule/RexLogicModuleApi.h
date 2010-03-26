// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_RexLogicModuleApi_h
#define incl_RexLogicModuleApi_h

#if defined (_WINDOWS)
#if defined(REXLOGIC_MODULE_EXPORTS)
#define REXLOGIC_MODULE_API __declspec(dllexport)
#else
#define REXLOGIC_MODULE_API __declspec(dllimport)
#endif
#else
#define REXLOGIC_MODULE_API
#endif

#endif

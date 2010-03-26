// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_ConsoleModuleApi_h
#define incl_ConsoleModuleApi_h

#if defined (_WINDOWS)
#if defined(CONSOLE_MODULE_EXPORTS) 
#define CONSOLE_MODULE_API __declspec(dllexport)
#else
#define CONSOLE_MODULE_API __declspec(dllimport) 
#endif
#else
#define CONSOLE_MODULE_API
#endif

#endif

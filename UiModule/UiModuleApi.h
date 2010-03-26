// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModuleApi_h
#define incl_UiModuleApi_h

#if defined (_WINDOWS)
#if defined(UI_MODULE_EXPORTS) 
#define UI_MODULE_API __declspec(dllexport)
#else
#define UI_MODULE_API __declspec(dllimport) 
#endif
#else
#define UI_MODULE_API
#endif

#endif // incl_UiModuleApi_h

// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_QtModuleApi_h
#define incl_QtModuleApi_h

#if defined (_WINDOWS)
#if defined(QT_MODULE_EXPORTS) 
#define QT_MODULE_API __declspec(dllexport)
#else
#define QT_MODULE_API __declspec(dllimport) 
#endif
#else
#define QT_MODULE_API
#endif

#endif

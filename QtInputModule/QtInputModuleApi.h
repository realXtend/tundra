// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_QtInputModule_h
#define incl_QtInputModule_h

#if defined (_WINDOWS)
#if defined(QTINPUT_MODULE_EXPORTS) 
#define QTINPUT_MODULE_API __declspec(dllexport)
#else
#define QTINPUT_MODULE_API __declspec(dllimport) 
#endif
#else
#define QTINPUT_MODULE_API
#endif

#endif

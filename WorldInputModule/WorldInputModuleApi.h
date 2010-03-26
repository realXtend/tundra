// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_WorldInputModule_h
#define incl_WorldInputModule_h

#if defined (_WINDOWS)
#if defined(WORLDINPUT_MODULE_EXPORTS) 
#define WORLDINPUT_MODULE_API __declspec(dllexport)
#else
#define WORLDINPUT_MODULE_API __declspec(dllimport) 
#endif
#else
#define WORLDINPUT_MODULE_API
#endif

#endif

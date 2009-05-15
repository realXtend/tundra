// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_GtkmmUIApi_h
#define incl_GtkmmUIApi_h

#if defined (_WINDOWS)
#if defined(GTKMMUI_MODULE_EXPORTS) 
#define GTKMMUI_MODULE_API __declspec(dllexport)
#else
#define GTKMMUI_MODULE_API __declspec(dllimport) 
#endif
#else
#define GTKMMUI_MODULE_API
#endif

#endif

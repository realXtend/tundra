// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_ECEditorModuleApi_h
#define incl_ECEditorModuleApi_h

#if defined (_WINDOWS)
#if defined(ECEDITOR_MODULE_EXPORTS)
#define ECEDITOR_MODULE_API __declspec(dllexport)
#else
#define ECEDITOR_MODULE_API __declspec(dllimport)
#endif
#else
#define ECEDITOR_MODULE_API
#endif

#endif

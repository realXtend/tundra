//$ HEADER_NEW_FILE $
#ifndef incl_UiExternalModuleApi_h
#define incl_UiExternalModuleApi_h

#if defined (_WINDOWS)
#if defined(UIEXTERNAL_MODULE_EXPORTS) 
#define UIEXTERNAL_MODULE_API __declspec(dllexport)
#else
#define UIEXTERNAL_MODULE_API __declspec(dllimport) 
#endif
#else
#define UIEXTERNAL_MODULE_API
#endif

#endif // incl_UiExternalModuleApi_h

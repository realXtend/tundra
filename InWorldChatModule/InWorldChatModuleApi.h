// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_InWorldChatModuleApi_h
#define incl_InWorldChatModuleApi_h

#if defined (_WINDOWS)
#if defined(INWORLDCHAT_MODULE_EXPORTS) 
#define INWORLDCHAT_MODULE_API __declspec(dllexport)
#else
#define INWORLDCHAT_MODULE_API __declspec(dllimport) 
#endif
#else
#define INWORLDCHAT_MODULE_API
#endif

#endif

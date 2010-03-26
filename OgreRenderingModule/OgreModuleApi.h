// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OgreModuleApi_h
#define incl_OgreModuleApi_h

#if defined (_WINDOWS)
#if defined(OGRE_MODULE_EXPORTS) 
#define OGRE_MODULE_API __declspec(dllexport)
#else
#define OGRE_MODULE_API __declspec(dllimport) 
#endif
#else
#define OGRE_MODULE_API
#endif

#endif

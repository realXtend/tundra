// For conditions of distribution and use, see copyright notice in license.txt

#pragma once

#if defined (_WINDOWS)
#if defined(OGRE_MODULE_EXPORTS) 
#define OGRE_MODULE_API __declspec(dllexport)
#else
#define OGRE_MODULE_API __declspec(dllimport) 
#endif
#else
#define OGRE_MODULE_API
#endif


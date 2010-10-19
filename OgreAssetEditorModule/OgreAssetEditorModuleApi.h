// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OgreAssetEditorModuleApi_OgreAssetEditorModuleApi_h
#define incl_OgreAssetEditorModuleApi_OgreAssetEditorModuleApi_h

#if defined (_WINDOWS)
#if defined(ASSET_EDITOR_MODULE_EXPORTS) 
#define ASSET_EDITOR_MODULE_API __declspec(dllexport)
#else
#define ASSET_EDITOR_MODULE_API __declspec(dllimport) 
#endif
#else
#define ASSET_EDITOR_MODULE_API
#endif

#endif

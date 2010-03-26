// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OpenALAudio_OpenALAudioModuleApi_h
#define incl_OpenALAudio_OpenALAudioModuleApi_h

#if defined (_WINDOWS)
#if defined(OPENAL_MODULE_EXPORTS)
#define OPENAL_MODULE_API __declspec(dllexport)
#else
#define OPENAL_MODULE_API __declspec(dllimport)
#endif
#else
#define OPENAL_MODULE_API
#endif

#endif

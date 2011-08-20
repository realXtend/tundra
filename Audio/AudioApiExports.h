// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_AudioApi_h
#define incl_AudioApi_h

#if defined (_WINDOWS)
#if defined(AUDIO_API_EXPORTS)
#define AUDIO_API __declspec(dllexport)
#else
#define AUDIO_API __declspec(dllimport)
#endif
#else
#define AUDIO_API
#endif

#endif

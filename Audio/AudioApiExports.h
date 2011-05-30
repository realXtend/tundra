// For conditions of distribution and use, see copyright notice in license.txt

#pragma once

#if defined (_WINDOWS) && defined(AUDIO_API_DLL)
#if defined(AUDIO_API_EXPORTS)
#define AUDIO_API __declspec(dllexport)
#else
#define AUDIO_API __declspec(dllimport)
#endif
#else
#define AUDIO_API
#endif


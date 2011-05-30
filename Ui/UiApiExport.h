// For conditions of distribution and use, see copyright notice in license.txt

#pragma once

#if defined (_WINDOWS) && defined(UI_API_DLL)
#if defined(UI_API_EXPORTS)
#define UI_API __declspec(dllexport)
#else
#define UI_API __declspec(dllimport)
#endif
#else
#define UI_API
#endif


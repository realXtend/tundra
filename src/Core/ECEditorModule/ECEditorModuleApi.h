// For conditions of distribution and use, see copyright notice in license.txt

#pragma once

#if defined (_WINDOWS)
#if defined(ECEDITOR_MODULE_EXPORTS)
#define ECEDITOR_MODULE_API __declspec(dllexport)
#else
#define ECEDITOR_MODULE_API __declspec(dllimport)
#endif
#else
#define ECEDITOR_MODULE_API
#endif


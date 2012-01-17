// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#if defined (_WINDOWS)
#if defined(PYMODULE_MODULE_EXPORTS) 
#define PY_MODULE_API __declspec(dllexport)
#else
#define PY_MODULE_API __declspec(dllimport) 
#endif
#else
#define PY_MODULE_API
#endif

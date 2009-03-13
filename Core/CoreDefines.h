// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_CoreDefines_h
#define incl_CoreDefines_h

// useful defines

#define SAFE_DELETE(p) if (p) { delete p; p=NULL; }
#define SAFE_DELETE_ARRAY(p) if (p) { delete [] p; p=NULL; }

#define UNREFERENCED_PARAM(P)               (P)

#undef REX_API
#if defined (_WINDOWS)
#if defined(MODULE_EXPORTS) 
#define REX_API __declspec(dllexport) 
#else
#define REX_API __declspec(dllimport) 
#endif
#endif

#ifndef REX_API
#define REX_API
#endif

#endif


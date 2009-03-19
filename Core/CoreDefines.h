// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_CoreDefines_h
#define incl_CoreDefines_h

// useful defines

#define SAFE_DELETE(p) { delete p; p=NULL; }
#define SAFE_DELETE_ARRAY(p) { delete [] p; p=NULL; }

#define UNREFERENCED_PARAM(P)               (P)

#define TO_STRING(p) boost::lexical_cast<std::string>(p)

#undef MODULE_API
#if defined (_WINDOWS)
#if defined(MODULE_EXPORTS) 
#define MODULE_API __declspec(dllexport) 
#else
#define MODULE_API __declspec(dllimport) 
#endif
#endif

#ifndef MODULE_API
#define MODULE_API
#endif

#endif


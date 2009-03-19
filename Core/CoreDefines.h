// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_CoreDefines_h
#define incl_CoreDefines_h

// useful defines

#define SAFE_DELETE(p) { delete p; p=NULL; }
#define SAFE_DELETE_ARRAY(p) { delete [] p; p=NULL; }

#define UNREFERENCED_PARAM(P)               (P)

#define TO_STRING(p) boost::lexical_cast<std::string>(p)

#undef VIEWER_API
#if defined (_WINDOWS)
#if defined(MODULE_EXPORTS) 
#define VIEWER_API __declspec(dllexport) 
#else
#define VIEWER_API __declspec(dllimport) 
#endif
#endif

#ifndef VIEWER_API
#define VIEWER_API
#endif

#endif


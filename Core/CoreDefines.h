// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_CoreDefines_h
#define incl_CoreDefines_h

// useful defines

#define SAFE_DELETE(p) if (p) { delete p; p=NULL; }
#define SAFE_DELETE_ARRAY(p) if (p) { delete [] p; p=NULL; }

#define UNREFERENCED_PARAM(P)               (P)

#define LOG(x) std::cout << x << std::endl;
#define LOGWARNING(x) std::cout << "Warning: " << x << std::endl;
#define LOGERROR(x) std::cerr << "Error: " << x << std::endl;

#endif


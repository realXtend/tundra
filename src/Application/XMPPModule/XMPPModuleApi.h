// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_XMPPModule_h
#define incl_XMPPModule_h

#if defined (_WINDOWS)
    #if defined(XMPP_MODULE_EXPORTS)
        #define XMPP_MODULE_API __declspec(dllexport)
    #else
        #define XMPP_MODULE_API __declspec(dllimport)
    #endif
#else
    #define XMPP_MODULE_API
#endif

#endif

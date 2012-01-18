// For conditions of distribution and use, see copyright notice in license.txt

#pragma once

#if defined (_WINDOWS)
    #if defined(BrowserUiPlugin_EXPORTS) 
        #define BROWSERUI_MODULE_API __declspec(dllexport)
    #else
        #define BROWSERUI_MODULE_API __declspec(dllimport) 
    #endif
#else
    #define BROWSERUI_MODULE_API
#endif


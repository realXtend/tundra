// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_CorePlatform_h
#define incl_CorePlatform_h

#include "CorePlatformWin.h"
#include "CorePlatformNix.h"

namespace Core
{
    //! Low-level platform specific functionality
    class Platform : 
#if defined(WIN32) || defined(WIN64)
        public PlatformWin
#else
        public PlatformNix
#endif
    {
    public:
        //! default constructor
        Platform() {}
        //! destructor
        virtual ~Platform() {}
    };
}

#endif




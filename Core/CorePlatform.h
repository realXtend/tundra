// For conditions of distribution and use, see copyright notice in license.txt

#ifndef __incl_CorePlatform_h__
#define __incl_CorePlatform_h__

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




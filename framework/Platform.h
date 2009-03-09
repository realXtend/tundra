// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_FoundationPlatform_h
#define incl_FoundationPlatform_h

#include "PlatformWin.h"
#include "PlatformNix.h"

namespace Foundation
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




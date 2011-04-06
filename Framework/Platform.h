// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_FoundationPlatform_h
#define incl_FoundationPlatform_h

#include <boost/filesystem.hpp>

#include "PlatformWin.h"
#include "PlatformNix.h"

namespace Foundation
{
    //! Low-level platform specific functionality
    /*!
        See slso PlatformWin or PlatformNix.

        \ingroup Foundation_group
    */
    class Platform : 
#if defined(_WINDOWS)
        public PlatformWin
#else
        public PlatformNix
#endif
    {
    public:
        //! default constructor
        Platform(Framework *framework) :
#if defined(_WINDOWS)
        PlatformWin(framework)
#else
        PlatformNix(framework)
#endif
        {}

        //! destructor
        virtual ~Platform() {}

        //! Creates application data directory, if one doesn't already exist. Should be called before any data is handled.
        void PrepareApplicationDataDirectory()
        {
            // exceptions are not handled, since if this fails the application is more or less SOL.

            boost::filesystem::wpath path(GetApplicationDataDirectoryW());
            if (boost::filesystem::exists(path) == false)
            {
                boost::filesystem::create_directory(path);
            }

            path = boost::filesystem::wpath(GetUserDocumentsDirectoryW());
            if (boost::filesystem::exists(path) == false)
            {
                boost::filesystem::create_directory(path);
            }
        }
    };
}

#endif




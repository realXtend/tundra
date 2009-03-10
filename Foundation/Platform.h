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

        //! Creates application data directory, if one doesn't already exist. Should be called before any data is handled.
        static void PrepareApplicationDataDirectory()
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




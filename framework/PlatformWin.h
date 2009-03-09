// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_FoundationPlatformWin_h
#define incl_FoundationPlatformWin_h

#if defined(WIN32) || defined(WIN64)

#include <shlobj.h>

namespace Foundation
{
    //! Low-level Windows specific functionality
    class PlatformWin 
    {
    public:
        //! default constructor
        PlatformWin() {}
        //! destructor
        virtual ~PlatformWin() {}

        //! Displays a message to the user. Should be used when there is no usable window for displaying messages to the user.
        static void Message(const std::string& title, const std::string& text)
        {
            MessageBoxA( NULL, text.c_str(), title.c_str(), MB_OK | MB_ICONERROR | MB_TASKMODAL);
        }

        //! Displays a message to the user. Should be used when there is no usable window for displaying messages to the user.
        static void Message(const std::wstring& title, const std::wstring& text)
        {
            MessageBoxW( NULL, text.c_str(), title.c_str(), MB_OK | MB_ICONERROR | MB_TASKMODAL);
        }

        //! Returns user specific application data directory.
        /*! Returns non-unicode path. May throw an expection if folder is not found.
            
            Should be used for mutable data the user doesn't need to see / handle directly,
            such as the cache.
        */
        static std::string GetApplicationDataDirectory();

        //! Returns user specific documents directory.
        /*! Returns unicode path. May throw an expection if folder is not found.

            Should be used for mutable data the user doesn't need to see / handle directly,
            such as the cache.
        */
        static std::wstring GetApplicationDataDirectoryW();

        //! Returns user specific documents directory.
        /*! Returns non-unicode path. May throw an expection if folder is not found.

            Should be used for mutable data the user may want to access directly
            at some point, possibly log files, screenshots and so on.
        */
        static std::string GetUserDocumentsDirectory();

        //! Returns user specific documents directory.
        /*! Returns unicode path. May throw an expection if folder is not found.

            Should be used for mutable data the user may want to access directly
            at some point, possibly log files, screenshots and so on.
        */
        static std::wstring GetUserDocumentsDirectoryW();
    };
}
#endif

#endif




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
        */
        static std::string GetApplicationDataDirectory()
        {
            PIDLIST_ABSOLUTE pidl;

            //SHGetSpecialFolderLocation(NULL, CSIDL_APPDATA | CSIDL_FLAG_CREATE, &pidl);
            if (SHGetFolderLocation(NULL, CSIDL_APPDATA | CSIDL_FLAG_CREATE, NULL, 0, &pidl) == S_OK)
            {
                char cpath[MAX_PATH];
                SHGetPathFromIDListA( pidl, cpath );

                return std::string(cpath) + "\\" + Application::Name();
            }
            throw Core::Exception("Failed to access application data directory.");
        }

        //! Returns user specific application data directory.
        /*! Returns unicode path. May throw an expection if folder is not found.
        */
        static std::wstring GetApplicationDataDirectoryW()
        {
            PIDLIST_ABSOLUTE pidl;

            if (SHGetFolderLocation(NULL, CSIDL_APPDATA | CSIDL_FLAG_CREATE, NULL, 0, &pidl) == S_OK)
            {
                wchar_t cpath[MAX_PATH];
                SHGetPathFromIDListW( pidl, cpath );

                return std::wstring(cpath) + L"\\" + Application::NameW();
            }
            throw Core::Exception("Failed to access application data directory.");
        }
    };
}
#endif

#endif




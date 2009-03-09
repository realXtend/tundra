// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_FoundationPlatformNix_h
#define incl_FoundationPlatformNix_h

#if !defined(WIN32) && !defined(WIN64)
namespace Foundation
{
    //! Low-level *nix specific functionality
    class PlatformNix
    {
    public:
        //! default constructor
        PlatformNix() {}
        //! destructor
        virtual ~PlatformNix() {}

        static void Message(const std::string& title, const std::string& text)
        {
            std::cerr << title << " " << text;
        }

        static void Message(const std::wstring& title, const std::wstring& text)
        {
            std::wcerr << title << " " << text;
        }

        //! Returns user specific application data directory.
        /*! Returns non-unicode path. May throw an expection if folder is not found.
        */
        static std::string GetApplicationDataDirectory()
        {
            char *ppath = NULL;
            ppath = getenv("HOME");
            if (ppath == NULL)
                throw Core::Exception("Failed to get HOME environment variable.");

            std::string path(ppath);
            return path + "/." + Application::Name();
        }

        //! Returns user specific application data directory.
        /*! Returns unicode path. May throw an expection if folder is not found.
        */
        static std::wstring GetApplicationDataDirectoryW()
        {
            // Unicode not supported on Unix-based platforms

            std::string path = GetApplicationDataDirectory();
            std::wstring pathw(path.length(), L' ');
            std::copy(path.begin(), path.end(), pathw.begin());

            return (pathw);
        }
    };
}
#endif

#endif




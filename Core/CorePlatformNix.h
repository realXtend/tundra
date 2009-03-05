// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_CorePlatformNix_h
#define incl_CorePlatformNix_h

#if !defined(WIN32) && !defined(WIN64)
namespace Core
{
    //! Low-level *nix specific functionality
    class PlatformNix
    {
    public:
        //! default constructor
        PlatformNix() {}
        //! destructor
        virtual ~PlatformNix() {}

        static void message(const std::string& title, const std::string& text)
        {
            std::cerr << title << " " << text;
        }

        static void message(const std::wstring& title, const std::wstring& text)
        {
            std::wcerr << title << " " << text;
        }
    };
}
#endif

#endif




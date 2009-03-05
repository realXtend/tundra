// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_CorePlatformWin_h
#define incl_CorePlatformWin_h

#if defined(WIN32) || defined(WIN64)
namespace Core
{
    //! Low-level Windows specific functionality
    class PlatformWin 
    {
    public:
        //! default constructor
        PlatformWin() {}
        //! destructor
        virtual ~PlatformWin() {}

        static void message(const std::string& title, const std::string& text)
        {
            MessageBoxA( NULL, text.c_str(), title.c_str(), MB_OK | MB_ICONERROR | MB_TASKMODAL);
        }

        static void message(const std::wstring& title, const std::wstring& text)
        {
            MessageBoxW( NULL, text.c_str(), title.c_str(), MB_OK | MB_ICONERROR | MB_TASKMODAL);
        }
    };
}
#endif

#endif




// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_FoundationPlatformWin_h
#define incl_FoundationPlatformWin_h

#if defined(_WINDOWS)



namespace Foundation
{
    class Framework;

    //! Low-level Windows specific functionality
    /*! \ingroup Foundation_group
    */
    class PlatformWin 
    {
    public:
        //! default constructor
        PlatformWin(Framework *framework) : framework_(framework) {}
        //! destructor
        virtual ~PlatformWin() {}

        //! Displays a message to the user. Should be used when there is no usable window for displaying messages to the user.
        static void Message(const std::string& title, const std::string& text);

        //! Displays a message to the user. Should be used when there is no usable window for displaying messages to the user.
        static void Message(const std::wstring& title, const std::wstring& text);

        //! Returns user specific application data directory.
        /*! Returns non-unicode path. May throw an expection if folder is not found.
            
            Should be used for mutable data the user doesn't need to see / handle directly,
            such as the cache.
        */
        std::string GetApplicationDataDirectory();

        //! Returns user specific documents directory as wide string.
        /*! Returns unicode path. May throw an expection if folder is not found.

            Should be used for mutable data the user doesn't need to see / handle directly,
            such as the cache.
        */
        std::wstring GetApplicationDataDirectoryW();

        //! Returns user specific documents directory.
        /*! Returns non-unicode path. May throw an expection if folder is not found.

            Should be used for mutable data the user may want to access directly
            at some point, possibly log files, screenshots and so on.
        */
        std::string GetUserDocumentsDirectory();

        //! Returns user specific documents directory as wide string.
        /*! Returns unicode path. May throw an expection if folder is not found.

            Should be used for mutable data the user may want to access directly
            at some point, possibly log files, screenshots and so on.
        */
        std::wstring GetUserDocumentsDirectoryW();

    private:
        Framework *framework_;
    };
}
#endif

#endif




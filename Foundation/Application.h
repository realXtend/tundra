// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Foundation_Application_h
#define incl_Foundation_Application_h

#include "CoreStringUtils.h"

namespace Foundation
{
    class Framework;

    //! shortcuts for accessing application specific data, such as name and version.
    /*! \ingroup Foundation_group
    */
    class Application : public boost::noncopyable
    {
        Application();

    public:
        //! constructor that takes a framework
        explicit Application(Framework *framework);

        //! destructor
        ~Application();

        //! shortcut for application name
        static std::string Name();

        //! shortcut for application name, returns wide string
        static std::wstring NameW() { return ToWString(Name()); }

        //! shortcut for application version string
        static std::string Version();

        //! shortcut for application version string, returns wide string
        static std::wstring VersionW() { return ToWString(Version()); }

    private:
        static Framework *framework_;
    };
}

#endif

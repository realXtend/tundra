// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Foundation_Application_h
#define incl_Foundation_Application_h

#include "Framework.h"

namespace Foundation
{
    //! shortcuts for accessing application specific data, such as name and version.
    class Application : public boost::noncopyable
    {
        Application();
    public:
        //! constructor that takes a framework
        Application(Framework *framework) { framework_ = framework; }

        //! destructor
        ~Application() { framework_ = NULL; }

        //! shortcut for application name
        static std::string Name()
        {
            assert (framework_);
            static const std::string app_name("application_name");

            return framework_->GetDefaultConfig().GetSetting<std::string>(Framework::ConfigurationGroup(), app_name);
        }

        //! shortcut for application name, returns wide string
        static std::wstring NameW()
        {
            return Core::ToWString(Name());
        }

        //! shortcut for application version string
        static std::string Version()
        {
            assert (framework_);
            static const std::string version_major("version_major");
            static const std::string version_minor("version_minor");
            const std::string &group = Framework::ConfigurationGroup();

            return ( framework_->GetDefaultConfig().GetSetting<std::string>(group, version_major) + "." +
                    framework_->GetDefaultConfig().GetSetting<std::string>(group, version_minor) );
        }

        //! shortcut for application version string, returns wide string
        static std::wstring VersionW()
        {
            return Core::ToWString(Version());
        }

    private:
        static Framework *framework_;
    };
}

#endif

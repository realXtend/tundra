// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_CoreApplicationName_h
#define incl_CoreApplicationName_h

#include "Framework.h"

namespace Foundation
{
    struct Application
    {
        //! Shortcut for returning the name of the application.
        /*! Same as retrieving it from the application configuration
            file.
        */
        static std::string Name()
        {
            ConfigurationManager &config = Framework::GetDefaultConfig();
            return config.DeclareSetting(Framework::ConfigurationGroup(), "application_name", "realXtend");
        }

        //! \copydoc Name()
        static std::wstring NameW()
        {
            std::string name = Name();
            std::wstring namew(name.length(), L' ');
            std::copy(name.begin(), name.end(), namew.begin());
            return namew;
        }
    };
}

#endif

// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "PlatformNix.h"
#include "Framework.h"
#include "ConfigurationManager.h"
#include "CoreStringUtils.h"
#include "CoreException.h"

#if !defined(_WINDOWS)

namespace Foundation
{
    std::string PlatformNix::GetApplicationDataDirectory()
    {
        char *ppath = 0;
        ppath = getenv("HOME");
        if (ppath == 0)
            throw Exception("Failed to get HOME environment variable.");

        std::string path(ppath);
        return path + "/." + framework_->GetDefaultConfig().GetSetting<std::string>(Framework::ConfigurationGroup(), "application_name");
    }

    std::wstring PlatformNix::GetApplicationDataDirectoryW()
    {
        // Unicode not supported on Unix-based platforms

        std::string path = GetApplicationDataDirectory();

        return (ToWString(path));
    }

    std::string PlatformNix::GetUserDocumentsDirectory()
    {
        return GetApplicationDataDirectory();
    }

    std::wstring PlatformNix::GetUserDocumentsDirectoryW()
    {
        return GetApplicationDataDirectoryW();
    }
}

#endif


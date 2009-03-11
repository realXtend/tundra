// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "PlatformWin.h"
#include "Framework.h"

#if defined(_WINDOWS)

namespace Foundation
{
    std::string PlatformWin::GetApplicationDataDirectory()
    {
        LPITEMIDLIST pidl;

        if (SHGetFolderLocation(NULL, CSIDL_APPDATA | CSIDL_FLAG_CREATE, NULL, 0, &pidl) == S_OK)
        {
            char cpath[MAX_PATH];
            SHGetPathFromIDListA( pidl, cpath );
            CoTaskMemFree(pidl);

            return std::string(cpath) + "\\" + framework_->GetDefaultConfig().GetString(Framework::ConfigurationGroup(), "application_name");
        }
        throw Core::Exception("Failed to access application data directory.");
    }

    std::wstring PlatformWin::GetApplicationDataDirectoryW()
    {
        LPITEMIDLIST pidl;

        if (SHGetFolderLocation(NULL, CSIDL_APPDATA | CSIDL_FLAG_CREATE, NULL, 0, &pidl) == S_OK)
        {
            wchar_t cpath[MAX_PATH];
            SHGetPathFromIDListW( pidl, cpath );
            CoTaskMemFree(pidl);

            return std::wstring(cpath) + L"\\" + Core::ToWString(framework_->GetDefaultConfig().GetString(Framework::ConfigurationGroup(), "application_name"));
        }
        throw Core::Exception("Failed to access application data directory.");
    }

    std::string PlatformWin::GetUserDocumentsDirectory()
    {
        LPITEMIDLIST pidl;

        if (SHGetFolderLocation(NULL, CSIDL_PERSONAL | CSIDL_FLAG_CREATE, NULL, 0, &pidl) == S_OK)
        {
            char cpath[MAX_PATH];
            SHGetPathFromIDListA( pidl, cpath );
            CoTaskMemFree(pidl);

            return std::string(cpath) + "\\" + framework_->GetDefaultConfig().GetString(Framework::ConfigurationGroup(), "application_name");
        }
        throw Core::Exception("Failed to access user documents directory.");
    }


    std::wstring PlatformWin::GetUserDocumentsDirectoryW()
    {
        LPITEMIDLIST pidl;

        int res = SHGetFolderLocation(NULL, CSIDL_PERSONAL | CSIDL_FLAG_CREATE, NULL, 0, &pidl);
        if (res == S_OK)
        {
            wchar_t cpath[MAX_PATH];
            SHGetPathFromIDListW( pidl, cpath );
            CoTaskMemFree(pidl);

            return std::wstring(cpath) + L"\\" + Core::ToWString(framework_->GetDefaultConfig().GetString(Framework::ConfigurationGroup(), "application_name"));
        }
        if (res == E_INVALIDARG)
        throw Core::Exception("Failed to access user documents directory.");
        else
           throw Core::Exception("Failed to access user documents directory.");
    }
}

#endif


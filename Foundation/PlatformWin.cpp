// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "PlatformWin.h"
#include "Framework.h"

#if defined(_WINDOWS)

#include <windows.h>
#include <shlobj.h>

namespace Foundation
{
    void PlatformWin::Message(const std::string& title, const std::string& text)
    {
        MessageBoxA( NULL, text.c_str(), title.c_str(), MB_OK | MB_ICONERROR | MB_TASKMODAL);
    }

    void PlatformWin::Message(const std::wstring& title, const std::wstring& text)
    {
        MessageBoxW( NULL, text.c_str(), title.c_str(), MB_OK | MB_ICONERROR | MB_TASKMODAL);
    }

    std::string PlatformWin::GetApplicationDataDirectory()
    {
        LPITEMIDLIST pidl;

        if (SHGetFolderLocation(NULL, CSIDL_APPDATA | CSIDL_FLAG_CREATE, NULL, 0, &pidl) == S_OK)
        {
            char cpath[MAX_PATH];
            SHGetPathFromIDListA( pidl, cpath );
            CoTaskMemFree(pidl);

            return std::string(cpath) + "\\" + framework_->GetDefaultConfig().GetSetting<std::string>(Framework::ConfigurationGroup(), "application_name");
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

            return std::wstring(cpath) + L"\\" + Core::ToWString(framework_->GetDefaultConfig().GetSetting<std::string>(Framework::ConfigurationGroup(), "application_name"));
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

            return std::string(cpath) + "\\" + framework_->GetDefaultConfig().GetSetting<std::string>(Framework::ConfigurationGroup(), "application_name");
        }
        throw Core::Exception("Failed to access user documents directory.");
    }


    std::wstring PlatformWin::GetUserDocumentsDirectoryW()
    {
        LPITEMIDLIST pidl;

        if (SHGetFolderLocation(NULL, CSIDL_PERSONAL | CSIDL_FLAG_CREATE, NULL, 0, &pidl) == S_OK)
        {
            wchar_t cpath[MAX_PATH];
            SHGetPathFromIDListW( pidl, cpath );
            CoTaskMemFree(pidl);

            return std::wstring(cpath) + L"\\" + Core::ToWString(framework_->GetDefaultConfig().GetSetting<std::string>(Framework::ConfigurationGroup(), "application_name"));
        }
        throw Core::Exception("Failed to access user documents directory.");
    }
}

#endif


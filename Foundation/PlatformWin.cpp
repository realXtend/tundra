// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "MemoryLeakCheck.h"
#include "PlatformWin.h"
#include "Framework.h"
#include "ConfigurationManager.h"
#include "CoreException.h"
#include "CoreStringUtils.h"

#if defined(_WINDOWS)

#include <QDebug>
#include <QApplication>
#include <QDir>

#include <windows.h>
#include <shlobj.h>

namespace Foundation
{
    void PlatformWin::Message(const std::string& title, const std::string& text)
    {
        MessageBoxA(0, text.c_str(), title.c_str(), MB_OK | MB_ICONERROR | MB_TASKMODAL);
    }

    void PlatformWin::Message(const std::wstring& title, const std::wstring& text)
    {
        MessageBoxW(0, text.c_str(), title.c_str(), MB_OK | MB_ICONERROR | MB_TASKMODAL);
    }

    std::string PlatformWin::GetInstallDirectory()
    {
        /*! 
            \todo Fix this path mess. Find a more sophisticated solution for going around
            the fact that visual studio runs the directory from the project folder even if working directory is set.
            Or just rewrite the travel up code with Qt so we dont have duplicate code here.
        */

        // This is more reliable when the executable is launched by another application
        // like a web browser via the tundra:// protocol links. 
        // The old code below will fail with those always.
        QString appDirPath = QApplication::applicationDirPath();
        if (!appDirPath.isEmpty())
        {
            // Check the modules folder, if not there let it 
            // fallback to the old code that travels folders up to find modules
            // The fallback code is only used when you run inside visual studio and then appDirPath
            // will be C:\tundra\Viewer\{RelWithDebInfo|Debug|Release}, for normal runs outside VC getting
            // the path from QApplication should always work.
            QDir appDir(appDirPath);
            if (appDir.exists("modules"))
                return appDirPath.toStdString();
        }

        char cpath[MAX_PATH];
        GetCurrentDirectoryA(MAX_PATH, cpath);
        // When running from a debugger, the current directory may in fact be the install directory.
        // Check for the presence of a "modules" directory (not the best way, but should do)
        std::string path(cpath);
        if (boost::filesystem::exists(path + "\\modules"))
            return path;
        
        // Otherwise get the module's filename, and extract path from it
        GetModuleFileNameA(0, cpath, MAX_PATH);
        int length = strlen(cpath);
        for (int i = length - 1; i >= 0; --i)
        {
            if (cpath[i] == '\\')
            {
                cpath[i + 1] = 0;
                break;
            }
        }
        
        path = std::string(cpath);
        return path;
    }
    
    std::wstring PlatformWin::GetInstallDirectoryW()
    {
        /*! 
            \todo fix this like above is done. Why we just cant do appDirPath.toStdWString() here
            is because we get a linker error. More here: http://developer.qt.nokia.com/forums/viewthread/3004
            But its not a problem atm as this function seems to never be called.
        */

        qWarning() << "PlatformWin::GetInstallDirectoryW() is not up to date, use PlatformWin::GetInstallDirectory() if possible.";

        wchar_t cpath[MAX_PATH];
        GetCurrentDirectoryW(MAX_PATH, cpath);
        // When running from a debugger, the current directory may in fact be the install directory.
        // Check for the presence of a "modules" directory (not the best way, but should do)
        std::wstring path(cpath);
        if (boost::filesystem::exists(path + L"\\modules"))
            return path;
        
        // Otherwise get the module's filename, and extract path from it
        GetModuleFileNameW(0, cpath, MAX_PATH);
        int length = wcslen(cpath);
        for (int i = length - 1; i >= 0; --i)
        {
            if (cpath[i] == L'\\')
            {
                cpath[i + 1] = 0;
                break;
            }
        }
        
        path = std::wstring(cpath);
        return path;
    }
    
    std::string PlatformWin::GetApplicationDataDirectory()
    {
        LPITEMIDLIST pidl;

        if (SHGetFolderLocation(0, CSIDL_APPDATA | CSIDL_FLAG_CREATE, NULL, 0, &pidl) == S_OK)
        {
            char cpath[MAX_PATH];
            SHGetPathFromIDListA( pidl, cpath );
            CoTaskMemFree(pidl);

            return std::string(cpath) + "\\" + std::string(APPLICATION_NAME);
        }
        throw Exception("Failed to access application data directory.");
    }

    std::wstring PlatformWin::GetApplicationDataDirectoryW()
    {
        LPITEMIDLIST pidl;

        if (SHGetFolderLocation(0, CSIDL_APPDATA | CSIDL_FLAG_CREATE, NULL, 0, &pidl) == S_OK)
        {
            wchar_t cpath[MAX_PATH];
            SHGetPathFromIDListW( pidl, cpath );
            CoTaskMemFree(pidl);

            return std::wstring(cpath) + L"\\" + ToWString(std::string(APPLICATION_NAME));
        }
        throw Exception("Failed to access application data directory.");
    }

    std::string PlatformWin::GetUserDocumentsDirectory()
    {
        LPITEMIDLIST pidl;

        if (SHGetFolderLocation(0, CSIDL_PERSONAL | CSIDL_FLAG_CREATE, NULL, 0, &pidl) == S_OK)
        {
            char cpath[MAX_PATH];
            SHGetPathFromIDListA( pidl, cpath );
            CoTaskMemFree(pidl);

            return std::string(cpath) + "\\" + std::string(APPLICATION_NAME);
        }
        throw Exception("Failed to access user documents directory.");
    }


    std::wstring PlatformWin::GetUserDocumentsDirectoryW()
    {
        LPITEMIDLIST pidl;

        if (SHGetFolderLocation(0, CSIDL_PERSONAL | CSIDL_FLAG_CREATE, NULL, 0, &pidl) == S_OK)
        {
            wchar_t cpath[MAX_PATH];
            SHGetPathFromIDListW( pidl, cpath );
            CoTaskMemFree(pidl);

            return std::wstring(cpath) + L"\\" + ToWString(std::string(APPLICATION_NAME));
        }
        throw Exception("Failed to access user documents directory.");
    }
}

#endif


// For conditions of distribution and use, see copyright notice in license.txt


#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif

#if defined(WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <string>
#include <boost/shared_ptr.hpp>

//#include "Core.h"
#include "Framework.h"


#if defined(_MSC_VER) && defined(_DMEMORYLEAKS)
// for reporting memory leaks upon debug exit
#include <crtdbg.h>
#endif

#if defined(_MSC_VER) && defined(_DMEMDUMP)
// For generating minidump
#  include <dbghelp.h>
#  include <shellapi.h>
#  include <shlobj.h>
#pragma warning( push )
#pragma warning( disable : 4996 )
#  include <strsafe.h>
#pragma warning( pop )
#endif

int run(void);
void message(const std::string& title, const std::string& text);
void messageU(const std::wstring& title, const std::wstring& text);
#if defined(_MSC_VER) && defined(_DMEMDUMP)
int generateDump(EXCEPTION_POINTERS* pExceptionPointers);
#endif

int main(int argc, char **argv)
{
    int retVal = EXIT_SUCCESS;

    // set debug flag for memory leaks
#   if defined(_MSC_VER) && defined(_DMEMORYLEAKS)
    int tmpDbgFlag;
    tmpDbgFlag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
    tmpDbgFlag |= _CRTDBG_LEAK_CHECK_DF;
    _CrtSetDbgFlag(tmpDbgFlag);
#   endif


#   if defined(_MSC_VER) && defined(_DMEMDUMP)
    __try
    {
#   endif

    retVal = run();

#   if defined(_MSC_VER) && defined(_DMEMDUMP)
    }
    __except(generateDump(GetExceptionInformation()))
    {
    }
#  endif

   return retVal;
}

int run(void)
{
    int retVal = EXIT_SUCCESS;

    // Create application object
    Foundation::Framework fw;
    try 
    {
        fw.go();
    } 
    catch ( std::exception& e )
    {
        message("An exception has occurred!", e.what());
        retVal = EXIT_FAILURE;
    }

    return retVal;
}

void message(const std::string& title, const std::string& text)
{
#  ifdef WIN32
      MessageBoxA( NULL, text.c_str(), title.c_str(), MB_OK | MB_ICONERROR | MB_TASKMODAL);
#  else
      std::cerr << title << " " << text;
#  endif
}

void messageU(const std::wstring& title, const std::wstring& text)
{
#  ifdef WIN32
      MessageBox( NULL, text.c_str(), title.c_str(), MB_OK | MB_ICONERROR | MB_TASKMODAL);
#  else
      std::cerr << title << " " << text;
#  endif
}

#if defined(_MSC_VER) && defined(_DMEMDUMP)
int generateDump(EXCEPTION_POINTERS* pExceptionPointers)
{
#define _CRT_SECURE_NO_WARNINGS
    BOOL bMiniDumpSuccessful;
    WCHAR szPath[MAX_PATH]; 
    WCHAR szFileName[MAX_PATH]; 
    WCHAR* szAppName = L"RexNG";
    WCHAR* szVersion = L"proto_demo";
    DWORD dwBufferSize = MAX_PATH;
    HANDLE hDumpFile;
    SYSTEMTIME stLocalTime;
    MINIDUMP_EXCEPTION_INFORMATION ExpParam;

    GetLocalTime( &stLocalTime );
    GetTempPath( dwBufferSize, szPath );

    StringCchPrintf( szFileName, MAX_PATH, L"%s%s", szPath, szAppName );
    CreateDirectoryW( szFileName, NULL );
    StringCchPrintf( szFileName, MAX_PATH, L"%s%s\\%s-%04d%02d%02d-%02d%02d%02d-%ld-%ld.dmp", 
               szPath, szAppName, szVersion, 
               stLocalTime.wYear, stLocalTime.wMonth, stLocalTime.wDay, 
               stLocalTime.wHour, stLocalTime.wMinute, stLocalTime.wSecond, 
               GetCurrentProcessId(), GetCurrentThreadId());

    hDumpFile = CreateFile(szFileName, GENERIC_READ|GENERIC_WRITE, 
                FILE_SHARE_WRITE|FILE_SHARE_READ, 0, CREATE_ALWAYS, 0, 0);

    ExpParam.ThreadId = GetCurrentThreadId();
    ExpParam.ExceptionPointers = pExceptionPointers;
    ExpParam.ClientPointers = TRUE;

    bMiniDumpSuccessful = MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), 
                    hDumpFile, MiniDumpWithDataSegs, &ExpParam, NULL, NULL);
   
    std::wstring message(L"Program ");
    message += szAppName;
    message += L" encountered an unexpected error.\n\nCrashdump was saved to location:\n";
    message += szFileName;

    if (bMiniDumpSuccessful)
        messageU(L"Minidump generated!", message);
    else
        messageU(szAppName, L"Un unexpected error was encountred while generating minidump!");

    return EXCEPTION_EXECUTE_HANDLER;
}
#endif



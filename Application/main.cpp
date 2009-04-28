// For conditions of distribution and use, see copyright notice in license.txt

#include "CoreStdIncludes.h"
#include "Core.h"
#include "Foundation.h"
#include "PreInit.h"


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
#if !defined(_DEBUG)
    try 
#endif
    {
        Foundation::Framework fw;
        PreInit init;
        init(&fw);

        fw.Go();
    } 
#if !defined(_DEBUG)
    catch ( std::exception& e )
    {
        Foundation::Platform::Message("An exception has occurred!", e.what());
        retVal = EXIT_FAILURE;
    }
#endif

    return retVal;
}


#if defined(_MSC_VER) && defined(_DMEMDUMP)
int generateDump(EXCEPTION_POINTERS* pExceptionPointers)
{
    BOOL bMiniDumpSuccessful;
    WCHAR szPath[MAX_PATH]; 
    WCHAR szFileName[MAX_PATH];

    // Can't use Foundation::Application for application name and version,
    // since it might have not been initialized yet, or it might have caused 
    // the exception in the first place
    WCHAR* szAppName = L"RealXtend";
    WCHAR* szVersion = L"pre-alpha-v-0-1";
    DWORD dwBufferSize = MAX_PATH;
    HANDLE hDumpFile;
    SYSTEMTIME stLocalTime;
    MINIDUMP_EXCEPTION_INFORMATION ExpParam;

    GetLocalTime( &stLocalTime );
    GetTempPathW( dwBufferSize, szPath );

    StringCchPrintf( szFileName, MAX_PATH, L"%s%s", szPath, szAppName );
    CreateDirectoryW( szFileName, NULL );
    StringCchPrintf( szFileName, MAX_PATH, L"%s%s\\%s-%04d%02d%02d-%02d%02d%02d-%ld-%ld.dmp", 
               szPath, szAppName, szVersion, 
               stLocalTime.wYear, stLocalTime.wMonth, stLocalTime.wDay, 
               stLocalTime.wHour, stLocalTime.wMinute, stLocalTime.wSecond, 
               GetCurrentProcessId(), GetCurrentThreadId());

    hDumpFile = CreateFileW(szFileName, GENERIC_READ|GENERIC_WRITE, 
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
        Foundation::Platform::Message(L"Minidump generated!", message);
    else
        Foundation::Platform::Message(szAppName, L"Un unexpected error was encountred while generating minidump!");

    return EXCEPTION_EXECUTE_HANDLER;
}
#endif



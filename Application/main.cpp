// For conditions of distribution and use, see copyright notice in license.txt

/*! \mainpage
 
    \section intro_sec Introduction
 
    Welcome to realXtend viewer source code documentation.
    
    First of all you might want to get to know coding conventions of the
    realXtend viewer. A document containing the guidelines can be found
    from the documentation directory.

    The viewer is divided into several different projects and namespaces.

    - The Core contains functionality common to all projects in the viewer.
    - The Foundation is the framework around which the viewer is build.
    - Several modules that create the functionality of the viewer with the
      help of the framework.
 
    \section install_sec Compiling the Viewer

    See readme.txt for information about compiling the viewer.

    \section license License

    For conditions of distribution and use, see copyright notice in license.txt

 

*/

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
    try 
    {
        Foundation::Framework fw;
        PreInit init;
        init(&fw);

        fw.Go();
    } 
    catch ( std::exception& e )
    {
#if defined(_DEBUG) && defined(_MSC_VER)
        // handling exception in debug mode. Feel free to modify if this doesn't work for you.
        UNREFERENCED_PARAM(e);
        __debugbreak();
#else
        Foundation::Platform::Message("An exception has occurred!", e.what());
        retVal = EXIT_FAILURE;
#endif
    }

    return retVal;
}


#if defined(_MSC_VER) && defined(_DMEMDUMP)
int generateDump(EXCEPTION_POINTERS* pExceptionPointers)
{
    BOOL bMiniDumpSuccessful;
    WCHAR szPath[MAX_PATH]; 
    WCHAR szFileName[MAX_PATH]; 
    WCHAR* szAppName = L"realXtend";
    WCHAR* szVersion = L"pre-alpha";
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



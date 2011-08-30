// For conditions of distribution and use, see copyright notice in license.txt

#include "DebugOperatorNew.h"

#include "Application.h"
#include "Framework.h"
#include "LoggingFunctions.h"
#include <QDir>

#ifdef _WIN32
#include <WinSock2.h>
#include <Windows.h>
#endif

#if defined(_MSC_VER) && defined(BUILDING_INSTALLER)
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <iostream>
#include <fstream>
#endif

#if defined(_MSC_VER) && defined(MEMORY_LEAK_CHECK)
// for reporting memory leaks upon debug exit
#include <crtdbg.h>
#endif

#if defined(_MSC_VER) && defined(_DMEMDUMP)
// For generating minidump
#include <dbghelp.h>
#include <shellapi.h>
#include <shlobj.h>
#pragma warning(push)
#pragma warning(disable : 4996)
#include <strsafe.h>
#pragma warning(pop)
#endif

#include "MemoryLeakCheck.h"

int run(int argc, char **argv);

#if defined(_MSC_VER) && defined(BUILDING_INSTALLER)
bool SpawnConsole();
#endif

#if defined(_MSC_VER) && defined(_DMEMDUMP)
int generate_dump(EXCEPTION_POINTERS* pExceptionPointers);
#endif

int main(int argc, char **argv)
{
    int return_value = EXIT_SUCCESS;

    // set up a debug flag for memory leaks. Output the results to file when the app exits.
    // Note that this file is written to the same directory where the executable resides,
    // so you can only use this in a development version where you have write access to
    // that directory.
#if defined(_MSC_VER) && defined(MEMORY_LEAK_CHECK) && defined(_DEBUG)
    int tmpDbgFlag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF;
    _CrtSetDbgFlag(tmpDbgFlag);

    HANDLE hLogFile = CreateFileW(L"fullmemoryleaklog.txt", GENERIC_WRITE, 
      FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
#endif

#if defined(_MSC_VER) && defined(_DMEMDUMP)
    __try
    {
#endif
        return_value = run(argc, argv);

#if defined(_MSC_VER) && defined(_DMEMDUMP)
    }
    __except(generate_dump(GetExceptionInformation()))
    {
    }
#endif

#if defined(_MSC_VER) && defined(MEMORY_LEAK_CHECK) && defined(_DEBUG)
    if (hLogFile != INVALID_HANDLE_VALUE)
    {
       _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
       _CrtSetReportFile(_CRT_WARN, hLogFile);
       _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
       _CrtSetReportFile(_CRT_ERROR, hLogFile);
       _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
       _CrtSetReportFile(_CRT_ASSERT, hLogFile);
    }
#endif

    // Note: We cannot close the file handle manually here. Have to let the OS close it
    // after it has printed out the list of leaks to the file.
    //CloseHandle(hLogFile);

    return return_value;
}

int run(int argc, char **argv)
{
    int return_value = EXIT_SUCCESS;
    int argcStartIndex = 1;

#if defined(_MSC_VER) && defined(BUILDING_INSTALLER)
    // The WinMain function does not get the executable path as first param,
    // so start checking start params from index 0.
    argcStartIndex = 0;
#endif

    // Parse and print command arguments
    QStringList arguments;
    for(int i = argcStartIndex; i < argc; ++i)
        arguments << argv[i];
    QString fullArguments = arguments.join(" ");
    
#if defined(_MSC_VER) && defined(BUILDING_INSTALLER)
    // Spawn console in BUILDING_INSTALLER mode if --server or --headless
    // 'spawnedConsole' marks if console should be freed on exit.
    bool spawnedConsole = false;
    if (arguments.contains("--server", Qt::CaseInsensitive) || arguments.contains("--headless", Qt::CaseInsensitive))
        spawnedConsole = SpawnConsole();
#endif

    // Print command arguments
    LogInfo("Starting up Tundra");
    LogInfo("* Working directory: " + QDir::currentPath());
    if (fullArguments.contains("--"))
    {
        LogInfo("* Command arguments:");
        int iStart = fullArguments.indexOf("--");
        while (iStart != -1)
        {       
            int iStop = fullArguments.indexOf("--", iStart+1);
            QString subStr = fullArguments.mid(iStart, iStop-iStart);
            if (!subStr.isEmpty() && !subStr.isNull())
            {
                LogInfo("  " + subStr);
                iStart = fullArguments.indexOf("--", iStart+1);
            }
            else
                iStart = -1;
        }
    }   
    LogInfo(""); // endl

    // Create application object
#if !defined(_DEBUG) || !defined (_MSC_VER)
    try
#endif
    {
        Framework* fw = new Framework(argc, argv);
        fw->Go();
        delete fw;
    }
#if !defined(_DEBUG) || !defined (_MSC_VER)
    catch(std::exception& e)
    {
        Application::Message("An exception has occurred!", e.what());
#if defined(_DEBUG)
        throw;
#else
        return_value = EXIT_FAILURE;
#endif
    }
#endif

// Free console if one was spawned
#if defined(_MSC_VER) && defined(BUILDING_INSTALLER)
    if (spawnedConsole)
        FreeConsole();
#endif

    return return_value;
}

#if defined(_MSC_VER) && defined(BUILDING_INSTALLER)
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    // Parse Windows command line
    std::vector<std::string> arguments;
    std::string cmdLine(lpCmdLine);
    unsigned i;
    unsigned cmdStart = 0;
    unsigned cmdEnd = 0;
    bool cmd = false;
    bool quote = false;

    for(i = 0; i < cmdLine.length(); ++i)
    {
        if (cmdLine[i] == '\"')
            quote = !quote;
        if ((cmdLine[i] == ' ') && (!quote))
        {
            if (cmd)
            {
                cmd = false;
                cmdEnd = i;
                arguments.push_back(cmdLine.substr(cmdStart, cmdEnd-cmdStart));
            }
        }
        else
        {
            if (!cmd)
            {
               cmd = true;
               cmdStart = i;
            }
        }
    }
    if (cmd)
        arguments.push_back(cmdLine.substr(cmdStart, i-cmdStart));
    
    std::vector<const char*> argv;
    for(unsigned int i = 0; i < arguments.size(); ++i)
        argv.push_back(arguments[i].c_str());
    
    if (argv.size())
        return main(argv.size(), (char**)&argv[0]);
    else
        return main(0, 0);
}

bool SpawnConsole()
{
    // Copied from http://dslweb.nwnexus.com/~ast/dload/guicon.htm
    // Simple AttachConsole() did not direct stdin/out to it.

    const WORD MAX_CONSOLE_LINES = 2000;
    int hConHandle;
    long lStdHandle;
    CONSOLE_SCREEN_BUFFER_INFO coninfo;
    FILE *fp;

    // Allocate a console for this app
    if (!AllocConsole())
        return false;

    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &coninfo);
    coninfo.dwSize.Y = MAX_CONSOLE_LINES;
    SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), coninfo.dwSize);

    // STDOUT
    lStdHandle = (long)GetStdHandle(STD_OUTPUT_HANDLE);
    hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
    fp = _fdopen( hConHandle, "w" );
    *stdout = *fp;
    setvbuf(stdout, NULL, _IONBF, 0);

    // STDIN
    lStdHandle = (long)GetStdHandle(STD_INPUT_HANDLE);
    hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
    fp = _fdopen( hConHandle, "r" );
    *stdin = *fp;
    setvbuf(stdin, NULL, _IONBF, 0);

    // STDERR
    lStdHandle = (long)GetStdHandle(STD_ERROR_HANDLE);
    hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
    fp = _fdopen( hConHandle, "w" );
    *stderr = *fp;
    setvbuf( stderr, NULL, _IONBF, 0 );

    // make cout, wcout, cin, wcin, wcerr, cerr, wclog and clog point to console as well
    std::ios::sync_with_stdio();

    return true;
}
#endif

#if defined(_MSC_VER) && defined(_DMEMDUMP)
int generate_dump(EXCEPTION_POINTERS* pExceptionPointers)
{
    // Add a hardcoded check to guarantee we only write a dump file of the first crash exception that is received.
    // Sometimes a crash is so bad that writing the dump below causes another exception to occur, in which case
    // this function would be recursively called, spawning tons of error dialogs to the user.
    static bool dumpGenerated = false;
    if (dumpGenerated)
    {
        printf("WARNING: Not generating another dump, one has been generated already!\n");
        return 0;
    }
    dumpGenerated = true;

    BOOL bMiniDumpSuccessful;
    WCHAR szPath[MAX_PATH]; 
    WCHAR szFileName[MAX_PATH];

    // Can't use Application for application name and version,
    // since it might have not been initialized yet, or it might have caused 
    // the exception in the first place
    WCHAR* szAppName = L"realXtend";
    WCHAR* szVersion = L"Tundra_v2.0";
    DWORD dwBufferSize = MAX_PATH;
    HANDLE hDumpFile;
    SYSTEMTIME stLocalTime;
    MINIDUMP_EXCEPTION_INFORMATION ExpParam;

    GetLocalTime( &stLocalTime );
    GetTempPathW( dwBufferSize, szPath );

    StringCchPrintf( szFileName, MAX_PATH, L"%s%s", szPath, szAppName );
    CreateDirectoryW( szFileName, 0 );
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
                    hDumpFile, MiniDumpWithDataSegs, &ExpParam, 0, 0);
   
    std::wstring message(L"Program ");
    message += szAppName;
    message += L" encountered an unexpected error.\n\nCrashdump was saved to location:\n";
    message += szFileName;

    if (bMiniDumpSuccessful)
        Application::Message(L"Minidump generated!", message);
    else
        Application::Message(szAppName, L"Unexpected error was encountered while generating minidump!");

    return EXCEPTION_EXECUTE_HANDLER;
}
#endif

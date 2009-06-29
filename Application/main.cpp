// For conditions of distribution and use, see copyright notice in license.txt

#include "CoreStdIncludes.h"
#include "Core.h"
#include "Foundation.h"

#include "CommandManager.h"
#include "TestModuleB.h"

#include <boost/program_options.hpp>
#include <boost/tokenizer.hpp>


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

void setup (Foundation::Framework &fw);
int run (int argc, char **argv);
void options (int argc, char **argv, Foundation::Framework &fw);

#if defined(_MSC_VER) && defined(_DMEMDUMP)
int generate_dump(EXCEPTION_POINTERS* pExceptionPointers);
#endif

int global_argc;
char **global_argv;

int main (int argc, char **argv)
{
    global_argc = argc;
    global_argv = argv;

    int return_value = EXIT_SUCCESS;

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

    return_value = run(argc, argv);

#   if defined(_MSC_VER) && defined(_DMEMDUMP)
    }
    __except(generate_dump(GetExceptionInformation()))
    {
    }
#  endif

   return return_value;
}

void setup (Foundation::Framework &fw)
{
    fw.GetModuleManager()->ExcludeModule(Foundation::Module::MT_Test);
    fw.GetModuleManager()->ExcludeModule(Test::TestModuleB::NameStatic());
}

int run (int argc, char **argv)
{
    int return_value = EXIT_SUCCESS;

    // Create application object
#if !defined(_DEBUG) || !defined (_MSC_VER)
    try
#endif
    {
        Foundation::Framework fw;
        setup (fw);
        fw.ParseProgramOptions(argc, argv);

        // \todo Parse command line options for command loading
        //options (argc, argv, fw);

        fw.Go();
    }
#if !defined(_DEBUG) || !defined (_MSC_VER)
    catch (std::exception& e)
    {
        Foundation::Platform::Message("An exception has occurred!", e.what());
#   if defined(_DEBUG)
        throw;
#   else
        return_value = EXIT_FAILURE;
#   endif
    }
#endif

    return return_value;
}

void options (int argc, char **argv, Foundation::Framework &fw)
{
    using namespace boost::program_options;
    typedef boost::char_separator <char> separator;
    typedef boost::tokenizer <separator> tokenizer;
    
    options_description desc;
    variables_map options;

    desc.add_options()
        ("headless", "run viewer without rendering")
        ("script", value<std::string>(), "commands to run");

    store (parse_command_line (argc, argv, desc), options);
    notify (options);

    if (options.count ("script"))
    {
        //\todo We can't use the CommandManager until it's (pre)loaded
        //tokenizer tok ((options["script"].as <std::string> ()), separator (";"));
        //Console::CommandManager *cm 
        //    (fw.GetService <Console::CommandManager>
        //     (Foundation::Service::ST_ConsoleCommand));

        //for (tokenizer::iterator it (tok.begin()); it != tok.end(); ++it)
        //    cm-> QueueCommand (*it);
    }
}

#if defined(_MSC_VER) && defined(WINDOWS_APP)
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

    for (i = 0; i < cmdLine.length(); ++i)
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
    for (int i = 0; i < arguments.size(); ++i)
        argv.push_back(arguments[i].c_str());
    
    if (argv.size())
        return main(argv.size(), (char**)&argv[0]);
    else
        return main(0, NULL);
}
#endif

#if defined(_MSC_VER) && defined(_DMEMDUMP)
int generate_dump(EXCEPTION_POINTERS* pExceptionPointers)
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



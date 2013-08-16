// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "Application.h"
#include "Framework.h"
#include "ConfigAPI.h"
#include "Profiler.h"
#include "CoreStringUtils.h"
#include "CoreException.h"
#include "LoggingFunctions.h"
#include "TundraVersionInfo.h"

#include <iostream>
#include <utility>

#include <QDir>
#include <QGraphicsView>
#include <QTranslator>
#include <QLocale>
#include <QIcon>
#ifndef ANDROID
#include <QWebSettings>
#endif
#include <QSplashScreen>

#if defined(_WINDOWS)
#include "Win.h"
#include <shlobj.h>
#include <io.h>
#include <fcntl.h>
#include <conio.h>
#endif

#ifdef __APPLE__
#include <mach-o/dyld.h>
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

#ifdef __APPLE__
#include "UiAPI.h"
#include "UiMainWindow.h"
#endif

#if defined(_MSC_VER) && defined(MEMORY_LEAK_CHECK)
// for reporting memory leaks upon debug exit
#include <crtdbg.h>
#endif

#include "MemoryLeakCheck.h"

/// @note Modify these values from the root CMakeLists.txt if you are making a custom Tundra build.
const char *Application::organizationName = TUNDRA_ORGANIZATION_NAME;
const char *Application::applicationName = TUNDRA_APPLICATION_NAME;
const char *Application::version = TUNDRA_VERSION_STRING;

Application::Application(int &argc, char **argv) :
    QApplication(argc, argv),
    framework(0),
    appActivated(true),
    nativeTranslator(new QTranslator),
    appTranslator(new QTranslator),
    targetFpsLimit(60.0),
    splashScreen(0)
{
    targetFpsLimitWhenInactive = targetFpsLimit / 2.f;
    // Reflect our versioning information to Qt internals, if something tries to obtain it straight from there.
    QApplication::setOrganizationName(organizationName);
    QApplication::setApplicationName(applicationName);
    QApplication::setApplicationVersion(version);

    // Parse version number parts.
    QStringList numberList = QString(version).split('.');
    if (numberList.size() > 4)
        LogWarning("[Application]: More than 4 numbers given for application version. Ignoring extra numbers.");

    int i = 0;
    for(; i < numberList.size() && i < 4; ++i)
    {
        bool ok = false;
        uint versionNumber = numberList[i].trimmed().toUInt(&ok);
        if (!ok)
        {
            // Conversion failed. Is this the last number in the string? Maybe some kind of
            // postfix is used, "-RC1" or " RC1" or similar f.ex., so handle that.
            if (i == numberList.size() - 1)
            {
                QStringList lastNumber = numberList[i].split(QRegExp("[^0-9]"));
                if (!lastNumber.isEmpty())
                    versionNumber = lastNumber.first().trimmed().toUInt(&ok);
            }
        }

        // If conversion still not ok, default to 0.
        versionNumbers[i] = (ok ? versionNumber : 0);
    }

    // Guarantee trailing zeros.
    while(i < 4)
        versionNumbers[i++] = 0;

#ifdef Q_WS_MAC
    QDir::setCurrent(QCoreApplication::applicationDirPath());
#endif

    // Make sure that the required Tundra data directories exist.
    QDir path = UserDataDirectory();
    if (!path.exists())
        path.mkpath(".");

    path = UserDocumentsDirectory();
    if (!path.exists())
        path.mkpath(".");

    // Add <install_dir>/qtplugins for qt to search plugins
    QString runDirectory = QDir::fromNativeSeparators(InstallationDirectory() + "/qtplugins");
    addLibraryPath(runDirectory);
    // In headless mode, we create windows that are never shown.
    // Also, the user can open up debugging windows like the profiler or kNet network stats from the console,
    // so disable the whole application from closing when these are shut down.
    // For headful mode, we depend on explicitly checking on the closing of the main window, so we don't need
    // this flag in any case.
    setQuitOnLastWindowClosed(false);

    /// @todo This seems a bit odd here. Would there be a better place and could this be configurable at startup-/run-time?
#ifndef ANDROID
    QWebSettings::globalSettings()->setAttribute(QWebSettings::PluginsEnabled, true); //enable flash
#endif
}

Application::~Application()
{
    SAFE_DELETE(splashScreen);
    SAFE_DELETE(nativeTranslator);
    SAFE_DELETE(appTranslator);
}

void Application::Initialize(Framework *fw)
{
    framework = fw;

    QDir dir("data/translations/qt_native_translations");
    QStringList qmFiles = FindQmFiles(dir);

    // Search then that is there corresponding native translations for system locals.
    QString loc = QLocale::system().name();
    loc.chop(3);

    QString name = "data/translations/qt_native_translations/qt_" + loc + ".qm";
    QStringList lst = qmFiles.filter(name);
    if (!lst.empty() )
        nativeTranslator->load(lst[0]);

    this->installTranslator(nativeTranslator);

    QString defaultLanguage = framework->Config()->DeclareSetting(ConfigAPI::FILE_FRAMEWORK,
        ConfigAPI::SECTION_FRAMEWORK, "language", "data/translations/tundra_en").toString();
    ChangeLanguage(defaultLanguage);

    ReadTargetFpsLimitFromConfig();
}

void Application::InitializeSplash()
{
    if (framework->IsHeadless())
        return;

    if (!splashScreen)
    {
        QString runDir = InstallationDirectory();
        splashScreen = new QSplashScreen(QPixmap(runDir + "/data/ui/images/adminotech_tundra_splash.png"));
        splashScreen->setFont(QFont("Calibri", 9));
        splashScreen->show();
        splashScreen->activateWindow();
#ifdef __APPLE__
        splashScreen->raise();
#endif
    }
}

void Application::SetSplashMessage(const QString &message)
{
    if (framework->IsHeadless())
        return;

    // Splash screen is enabled with --splash command.
    if (!framework->HasCommandLineParameter("--splash"))
        return;

    if (!splashScreen)
        InitializeSplash();

    if (splashScreen && splashScreen->isVisible())
    {
        // Call QApplication::processEvents() to update splash painting as at this point main loop is not running yet
        QString finalMessage = "v" + QString(Application::Version()) + " - " + message.toUpper();
        splashScreen->showMessage(finalMessage, Qt::AlignBottom|Qt::AlignLeft, QColor(25, 25, 25));
        processEvents();
    }
}

QStringList Application::FindQmFiles(const QDir& dir)
{
     QStringList fileNames = dir.entryList(QStringList("*.qm"), QDir::Files, QDir::Name);
     QMutableStringListIterator i(fileNames);
     while (i.hasNext())
     {
         i.next();
         i.setValue(dir.filePath(i.value()));
     }
     return fileNames;
}

void Application::Go()
{
    SAFE_DELETE(splashScreen);

#ifdef __APPLE__
    if (framework->Ui()->MainWindow())
    {
        framework->Ui()->MainWindow()->activateWindow();
        framework->Ui()->MainWindow()->raise();
    }
#endif

    installEventFilter(this);

    connect(&frameUpdateTimer, SIGNAL(timeout()), this, SLOT(UpdateFrame()));
    frameUpdateTimer.setSingleShot(true);
    frameUpdateTimer.start(0);

    try
    {
        exec();
    }
    catch(const std::exception &e)
    {
        std::string error("Application::Go caught an exception: " + std::string(e.what() ? e.what() : "(null)"));
        std::cout << error << std::endl;
        LogError(error);
        throw;
    }
    catch(...)
    {
        std::string error("Application::Go caught an unknown exception!");
        std::cout << error << std::endl;
        LogError(error);
        throw;
    }
}

void Application::Message(const char *title, const char *text)
{
#ifdef WIN32
    MessageBoxA(0, text != 0 ? text : "", title != 0 ? title : "", MB_OK | MB_ICONERROR | MB_TASKMODAL);
#else
    ///\todo The intention is to write a modal system message, but this simply prints to log.
    std::cerr << "Application::Message: " << title << ": " << text << std::endl;
#endif
}
void Application::Message(const std::string &title, const std::string &text)
{
    Message(title.c_str(), text.c_str());
}

void Application::Message(const wchar_t *title, const wchar_t *text)
{
#ifdef WIN32
    MessageBoxW(0, text != 0 ? text : L"", title != 0 ? title : L"", MB_OK | MB_ICONERROR | MB_TASKMODAL);
#else
    ///\todo The intention is to write a modal system message, but this simply prints to log.
    std::wcerr << L"Application::Message: " << title << L": " << text << std::endl;
#endif
}

void Application::Message(const std::wstring &title, const std::wstring &text)
{
    Message(title.c_str(), text.c_str());
}

bool Application::ShowConsoleWindow(bool attachToParent)
{
#ifdef WIN32
    BOOL success = 0;
    if (GetConsoleWindow())
        return true; // Console already exists.
    if (attachToParent)
        success = AttachConsole(ATTACH_PARENT_PROCESS);
    // Code below adapted from http://dslweb.nwnexus.com/~ast/dload/guicon.htm
    if (!success)
        success = AllocConsole();
    if (!success)
        return false;

    // Prepare stdin, stdout and stderr.
    long hStd =(long)GetStdHandle(STD_INPUT_HANDLE);
    int hCrt = _open_osfhandle(hStd, _O_TEXT);
    FILE *hf = _fdopen(hCrt, "r+");
    setvbuf(hf,0,_IONBF,0);
    *stdin = *hf;

    hStd =(long)GetStdHandle(STD_OUTPUT_HANDLE);
    hCrt = _open_osfhandle(hStd, _O_TEXT);
    hf = _fdopen(hCrt, "w+");
    setvbuf(hf, 0, _IONBF, 0);
    *stdout = *hf;

    hStd =(long)GetStdHandle(STD_ERROR_HANDLE);
    hCrt = _open_osfhandle(hStd, _O_TEXT);
    hf = _fdopen(hCrt, "w+");
    setvbuf(hf, 0, _IONBF, 0);
    *stderr = *hf;

    // Make C++ IO streams cout, wcout, cin, wcin, wcerr, cerr, wclog and clog point to console as well.
    std::ios::sync_with_stdio();
#endif

    return true;
}

void Application::SetCurrentWorkingDirectory(QString newCwd)
{
    bool successful = QDir::setCurrent(newCwd);
    assert(successful);
    UNREFERENCED_PARAM(successful);
}

QString Application::CurrentWorkingDirectory()
{
#ifdef _WINDOWS
    WCHAR str[MAX_PATH+1] = {};
    GetCurrentDirectoryW(MAX_PATH, str);
    QString qstr = WStringToQString(str);
#else
    QString qstr =  QDir::currentPath();
#endif
    if (!qstr.endsWith(QDir::separator()))
        qstr += QDir::separator();
    return qstr;
}

QString Application::InstallationDirectory()
{
    // When running from a debugger, the current directory may in fact be the install directory.
    // Check for the presence of a special tag file to see if we should treat cwd as the installation directory
    // instead of the directory where the current .exe resides.
    QString cwd = CurrentWorkingDirectory();
    if (QFile::exists(cwd + "plugins/TundraInstallationDirectory.txt"))
        return cwd;

#ifdef _WINDOWS
    WCHAR str[MAX_PATH+1] = {};
    DWORD success = GetModuleFileNameW(0, str, MAX_PATH);
    if (success == 0)
        return "";
    QString qstr = WStringToQString(str);
    // The module file name also contains the name of the executable, so strip it off.
    int trailingSlash = qstr.lastIndexOf('\\');
    if (trailingSlash == -1)
        return ""; // Some kind of error occurred.

    return qstr.left(trailingSlash+1); // +1 so that we return the trailing slash as well.
#elif defined(__APPLE__)
    char path[1024];
    uint32_t size = sizeof(path)-2;
    int ret = _NSGetExecutablePath(path, &size);
    if (ret == 0 && size > 0)
    {
        // The returned path also contains the executable name, so strip that off from the path name.
        QString p = path;
        int lastSlash = p.lastIndexOf("/");
        if (lastSlash != -1)
            p = p.left(lastSlash+1);
        return p;
    }
    else
    {
        LogError("Application::InstallationDirectory: _NSGetExecutablePath failed! Returning './'");
        return "./";
    }
#elif defined(ANDROID)
    /// \todo Implement a proper file access mechanism. Hardcoded internal storage access is used for now
    return "/sdcard/Download/Tundra/";
#elif defined(__linux__)
    char exeName[1024];
    memset(exeName, 0, 1024);
    pid_t pid = getpid();
    QString link = "/proc/" + QString::number(pid) + "/exe";
    readlink(link.toStdString().c_str(), exeName, 1024);
    // The returned path also contains the executable name, so strip that off from the path name.
    QString p(exeName);
    int lastSlash = p.lastIndexOf("/");
    if (lastSlash != -1)
        p = p.left(lastSlash+1);
    return p;     
#else
    LogError("Application::InstallationDirectory not implemented for this platform. Returning './'");
    return "./";
#endif
}

QString Application::UserDataDirectory()
{
    QString qstr;
#ifdef _WINDOWS
    LPITEMIDLIST pidl;

    if (SHGetFolderLocation(0, CSIDL_APPDATA | CSIDL_FLAG_CREATE, NULL, 0, &pidl) != S_OK)
        return "";

    WCHAR str[MAX_PATH+1] = {};
    SHGetPathFromIDListW(pidl, str);
    CoTaskMemFree(pidl);

    qstr = WStringToQString(str) + "\\" + ApplicationName();
#else
    ///\todo Convert to QString instead of std::string.
    char *ppath = 0;
    ppath = getenv("HOME");
    if (ppath == 0)
        throw Exception("Failed to get HOME environment variable.");

    qstr = QString(ppath) + "/." + ApplicationName();
#endif

    // Apply trailing slash
    if (!qstr.endsWith(QDir::separator()))
        qstr += QDir::separator();
    return qstr;
}

QString Application::UserDocumentsDirectory()
{
    QString qstr;
#ifdef _WINDOWS
    LPITEMIDLIST pidl;

    if (SHGetFolderLocation(0, CSIDL_PERSONAL | CSIDL_FLAG_CREATE, NULL, 0, &pidl) != S_OK)
        return "";

    WCHAR str[MAX_PATH+1] = {};
    SHGetPathFromIDListW(pidl, str);
    CoTaskMemFree(pidl);

    qstr = WStringToQString(str) + '\\' + ApplicationName();
#else
    ///\todo Review. Is this desirable?
    qstr = UserDataDirectory();
#endif

    // Apply trailing slash
    if (!qstr.endsWith(QDir::separator()))
        qstr += QDir::separator();
    return qstr;
}

QString Application::ParseWildCardFilename(const QString& input)
{
    // Parse all the special symbols from the log filename.
    QString filename = input.trimmed().replace("$(CWD)", CurrentWorkingDirectory(), Qt::CaseInsensitive);
    filename = filename.replace("$(INSTDIR)", InstallationDirectory(), Qt::CaseInsensitive);
    filename = filename.replace("$(USERDATA)", UserDataDirectory(), Qt::CaseInsensitive);
    filename = filename.replace("$(USERDOCS)", UserDocumentsDirectory(), Qt::CaseInsensitive);
    QRegExp rx("\\$\\(DATE:(.*)\\)");
    // Qt Regexes don't support non-greedy matching. The above regex should be "\\$\\(DATE:(.*?)\\)". Instead Qt supports
    // only setting the matching to be non-greedy globally.
    rx.setMinimal(true); // This is to avoid e.g. $(DATE:yyyyMMdd)_aaa).txt to be incorrectly captured as "yyyyMMdd)_aaa".
    for(;;) // Loop and find all instances of $(DATE:someformat).
    {
        int pos = rx.indexIn(filename);
        if (pos > -1)
        {
            QString dateFormat = rx.cap(1);
            QString date = QDateTime::currentDateTime().toString(dateFormat);
            filename = filename.replace(rx.pos(0), rx.cap(0).length(), date);
        }
        else
            break;
    }
    return filename;
}

const char *Application::OrganizationName()
{
    return organizationName;
}

const char *Application::ApplicationName()
{
    return applicationName;
}

const char *Application::Version()
{
    return version;
}

QString Application::FullIdentifier()
{
    return QString("%1 %2 %3").arg(organizationName).arg(applicationName).arg(version).trimmed();
}

void Application::ReadTargetFpsLimitFromConfig()
{
    ConfigData targetFpsConfigData(ConfigAPI::FILE_FRAMEWORK, ConfigAPI::SECTION_RENDERING);
    if (framework->Config()->HasValue(targetFpsConfigData, "fps target limit"))
    {
        bool ok;
        double targetFps = framework->Config()->Get(targetFpsConfigData, "fps target limit").toDouble(&ok);
        if (ok && targetFps >= 0.0)
            SetTargetFpsLimit(targetFps);
        else
            LogWarning("Application: Invalid target FPS value " + QString::number(targetFps) + " read from config. Ignoring.");
    }
}

bool Application::eventFilter(QObject *obj, QEvent *event)
{
    try
    {
        if (obj == this)
        {
            if (event->type() == QEvent::ApplicationActivate)
                appActivated = true;
            else if (event->type() == QEvent::ApplicationDeactivate)
                appActivated = false;
        }

        return QObject::eventFilter(obj, event);
    }
    catch(const std::exception &e)
    {
        std::string error("Application::eventFilter caught an exception: " + std::string(e.what() ? e.what() : "(null)"));
        std::cout << error << std::endl;
        LogError(error);
        throw;
    }
    catch(...)
    {
        std::string error("Application::eventFilter caught an unknown exception!");
        std::cout << error << std::endl;
        LogError(error);
        throw;
    }
}

void Application::ChangeLanguage(const QString& file)
{
    QString filename = file;
    if (!filename.endsWith(".qm", Qt::CaseInsensitive))
        filename.append(".qm");
    QString tmp = filename;
    tmp.chop(3);
    QString str = tmp.right(2);
    
    QString name = "data/translations/qt_native_translations/qt_" + str + ".qm";

    // Remove old translators then change them to new. 
    removeTranslator(nativeTranslator); 

    if (QFile::exists(name))
    {
        if (nativeTranslator != 0)
        {
            nativeTranslator->load(name);
            installTranslator(nativeTranslator); 
        }
    }
    else
    {
        if (nativeTranslator != 0 && nativeTranslator->isEmpty())
        {
            installTranslator(nativeTranslator);
        }
        else
        {
            SAFE_DELETE(nativeTranslator);
            nativeTranslator = new QTranslator;
            installTranslator(nativeTranslator); 
        }
    }

    // Remove old translators then change them to new. 
    removeTranslator(appTranslator);
    if (appTranslator->load(filename))
    {
        installTranslator(appTranslator);
        framework->Config()->Set(ConfigAPI::FILE_FRAMEWORK, ConfigAPI::SECTION_FRAMEWORK, "language", file);
    }
    
    emit LanguageChanged();
}

#ifdef __APPLE__
const int MAC_MIN_FONT_SIZE = 12;
void MakeFontsLargerOnOSX(QWidget *w);
#endif

bool Application::notify(QObject *receiver, QEvent *event)
{
    try
    {
#ifdef __APPLE__
        if (event->type() == QEvent::Polish && receiver && receiver->isWidgetType())
            MakeFontsLargerOnOSX(static_cast<QWidget*>(receiver));
#endif

        return QApplication::notify(receiver, event);
    }
    catch(const std::exception &e)
    {
        std::string error("Application::notify caught an exception: " + std::string(e.what() ? e.what() : "(null)"));
        std::cout << error << std::endl;
        LogError(error);
        throw;
    }
    catch(...)
    {
        std::string error("Application::notify caught an unknown exception!");
        std::cout << error << std::endl;
        LogError(error);
        throw;
    }
}

bool Application::IsActive() const
{
    return appActivated;
}

void Application::SetTargetFpsLimitWhenInactive(double fpsWhenInactive)
{
    if (fpsWhenInactive <= 0.0)
    {
        targetFpsLimitWhenInactive = targetFpsLimit;
        return;
    }
    else if (fpsWhenInactive > targetFpsLimit)
    {
        targetFpsLimitWhenInactive = targetFpsLimit / 2;
    }
    else
        targetFpsLimitWhenInactive = fpsWhenInactive;

    if (targetFpsLimitWhenInactive <= 1.0)
        targetFpsLimitWhenInactive = 0.0;
}

void Application::UpdateFrame()
{
    // Don't pump the QEvents to QApplication if we are exiting
    // also don't process our mainloop frames.
    if (framework->IsExiting())
        return;

    try
    {
        const tick_t frameStartTime = GetCurrentClockTime();

        QApplication::processEvents(QEventLoop::AllEvents, 1);
        QApplication::sendPostedEvents();

        framework->ProcessOneFrame();

        tick_t timeNow = GetCurrentClockTime();

        static tick_t timerFrequency = GetCurrentClockFreq();

        double msecsSpentInFrame = (double)(timeNow - frameStartTime) * 1000.0 / timerFrequency;

        const double msecsPerFrame = 1000.0 / (targetFpsLimit <= 1.0 ? 1000.0 : targetFpsLimit);
        double msecsPerFrameWhenInactive = 1000.0 / (targetFpsLimitWhenInactive <= 1.0 ? 1000.0 : targetFpsLimitWhenInactive);

        ///\note Ideally we should sleep 0 msecs when running at a high fps rate,
        /// but need to avoid QTimer::start() with 0 msecs, since that will cause the timer to immediately fire,
        /// which can cause the Win32 message loop inside Qt to starve. (Qt keeps spinning the timer.start(0) loop for Tundra mainloop and neglects Win32 API).
        double msecsToSleep = std::min(std::max(1.0, msecsPerFrame - msecsSpentInFrame), msecsPerFrame);
        double msecsToSleepWhenInactive = std::min(std::max(1.0, msecsPerFrameWhenInactive - msecsSpentInFrame), msecsPerFrameWhenInactive);

        // Reduce frame rate when unfocused
        if (!frameUpdateTimer.isActive())
        {
            if (appActivated || framework->IsHeadless())
                frameUpdateTimer.start((int)msecsToSleep);
            else
                frameUpdateTimer.start((int)(msecsToSleepWhenInactive)); // Cap FPS when window is inactive
        }
    }
    catch(const std::exception &e)
    {
        std::string error("Application::UpdateFrame caught an exception: " + std::string(e.what() ? e.what() : "(null)"));
        std::cout << error << std::endl;
        LogError(error);
        throw;
    }
    catch(...)
    {
        std::string error("Application::UpdateFrame caught an unknown exception!");
        std::cout << error << std::endl;
        LogError(error);
        throw;
    }
}

void Application::RequestExit()
{
    emit ExitRequested();

    // If no-one canceled the exit as a response to the signal, exit
    if (framework->IsExiting())
        quit();
}

QString Application::Platform()
{
#ifdef ANDROID
    return QString("android");
#elif defined(Q_WS_WIN)
    return QString("win");
#elif defined(Q_WS_MAC)
    return QString("mac");
#elif defined(Q_WS_X11)
    return QString("x11");
#else
    return QString();
#endif
}

#if defined(_MSC_VER) && defined(_DMEMDUMP)
int generate_dump(EXCEPTION_POINTERS* pExceptionPointers);
#endif

#if defined(_MSC_VER) && defined(MEMORY_LEAK_CHECK) && defined(_DEBUG)

int FilterMemoryLeaks(int reportType, char* message, int* retVal)
{
    static int allowLineCount = 0;
    
    // To make the report more readable, print only leaks that originate from own debug allocation functions
    if (strstr(message, ".cpp"))
        allowLineCount = 4;
    
    if (allowLineCount)
    {
        allowLineCount--;
        return FALSE;
    }
    else
        return TRUE;
}

void DumpMemoryLeaks()
{
    // Uncomment the next line to only print allocations that came through our debug allocation functions
    //_CrtSetReportHook(FilterMemoryLeaks);
    _CrtDumpMemoryLeaks();
}
#endif

int TUNDRACORE_API run(int argc, char **argv)
{
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
        int return_value = EXIT_SUCCESS;

        // Initialization prints
        LogInfo("Starting up " + Application::FullIdentifier());
        LogInfo("* Installation directory : " + Application::InstallationDirectory());
        LogInfo("* Working directory      : " + Application::CurrentWorkingDirectory());
        LogInfo("* User data directory    : " + Application::UserDataDirectory());

    // Create application object
#if !defined(_DEBUG) || !defined (_MSC_VER)
        try
#endif
        {
            Application app(argc, argv);
            Framework fw(argc, argv, &app);
            app.Initialize(&fw);
            fw.Go();
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
       
        // The DLL version of the CRT skips the call to _CrtDumpMemoryLeaks() at exit.
        // Add it as an atexit function instead. Note: we may get false positives due to static objects
        // and external libraries having not released their heap memory yet.
        atexit(DumpMemoryLeaks);
    }
#endif

    // Note: We cannot close the file handle manually here. Have to let the OS close it
    // after it has printed out the list of leaks to the file.
    //CloseHandle(hLogFile);

    return return_value;
}

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

    WCHAR szOrgName[MAX_PATH];
    WCHAR szAppName[MAX_PATH];
    WCHAR szVer[MAX_PATH];
    // Note: all the following Application functions access static const char * variables so it's safe to call them.
    MultiByteToWideChar(CP_ACP, 0, Application::OrganizationName(), -1, szOrgName, NUMELEMS(szOrgName));
    MultiByteToWideChar(CP_ACP, 0, Application::ApplicationName(), -1, szAppName, NUMELEMS(szAppName));
    MultiByteToWideChar(CP_ACP, 0, Application::Version(), -1, szVer, NUMELEMS(szVer));
    WCHAR szVersion[MAX_PATH]; // Will contain "<AppName>_v<Version>".
    StringCchPrintf(szVersion, MAX_PATH, L"%s_v%s", szAppName, szVer);

    DWORD dwBufferSize = MAX_PATH;
    HANDLE hDumpFile;
    SYSTEMTIME stLocalTime;
    MINIDUMP_EXCEPTION_INFORMATION ExpParam;

    GetLocalTime( &stLocalTime );
    GetTempPathW( dwBufferSize, szPath );

    StringCchPrintf(szFileName, MAX_PATH, L"%s%s", szPath, szOrgName/*szAppName*/);
    CreateDirectoryW(szFileName, 0);
    StringCchPrintf(szFileName, MAX_PATH, L"%s%s\\%s-%04d%02d%02d-%02d%02d%02d-%ld-%ld.dmp",
               szPath, szOrgName/*szAppName*/, szVersion,
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

    WCHAR szMessage[MAX_PATH];
    StringCchPrintf(szMessage, MAX_PATH, L"Program %s encountered an unexpected error.\n\nCrashdump was saved to location:\n%s", szAppName, szFileName);
    if (bMiniDumpSuccessful)
        Application::Message(L"Minidump generated!", szMessage);
    else
        Application::Message(szAppName, L"Unexpected error was encountered while generating minidump!");

    return EXCEPTION_EXECUTE_HANDLER;
}
#endif

#ifdef __APPLE__
void MakeFontsLargerOnOSX(QWidget *w)
{
    assert(w != 0);

    if (w->styleSheet() != "")
    {
        QRegExp fontSection("(font|font-size):\\s*(\\d+)(px|pt).*;");
        fontSection.setMinimal(true);
        int pos = fontSection.indexIn(w->styleSheet());
        int size = fontSection.capturedTexts()[2].toInt();
        if (size != 0 && size < MAC_MIN_FONT_SIZE)
        {
            QString fontProperty = fontSection.capturedTexts()[0];
            fontProperty.replace(fontSection.capturedTexts()[2] + fontSection.capturedTexts()[3],
                                 QString::number(MAC_MIN_FONT_SIZE) + fontSection.capturedTexts()[3]);
            QString stylesheet = w->styleSheet();
            stylesheet.replace(fontSection.capturedTexts()[0], fontProperty);
            w->setStyleSheet(stylesheet);
        }
    }
    else
    {
        if (w->font().pixelSize() != -1 && w->font().pixelSize() < MAC_MIN_FONT_SIZE)
            w->setStyleSheet("font-size: " + QString::number(MAC_MIN_FONT_SIZE) + "px;");
        else if (w->font().pointSize() != -1 && w->font().pointSize() < MAC_MIN_FONT_SIZE)
            w->setStyleSheet("font-size: " + QString::number(MAC_MIN_FONT_SIZE) + "pt;");
    }

}
#endif

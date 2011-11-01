// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "Application.h"
#include "Framework.h"
#include "VersionInfo.h"
#include "ConfigAPI.h"
#include "Profiler.h"
#include "CoreStringUtils.h"
#include "CoreException.h"
#include "LoggingFunctions.h"

#include <iostream>
#include <utility>

#include <QDir>
#include <QGraphicsView>
#include <QTranslator>
#include <QLocale>
#include <QIcon>
#include <QWebSettings>
#ifdef ENABLE_SPLASH_SCREEN
#include <QSplashScreen>
#endif

#ifdef Q_WS_MAC
#include <QMouseEvent>
#include <QWheelEvent>

#include "UiMainWindow.h"
#include "UiAPI.h"
#include "UiGraphicsView.h"
#endif

#if defined(_WINDOWS)
#include <WinSock2.h>
#include <windows.h>
#include <shlobj.h>
#undef min
#undef max
#endif

#include "MemoryLeakCheck.h"

/// @note Modify these values when you are making a custom Tundra build. Also the version needs to be changed here on releases.
const char *Application::organizationName = "realXtend";
const char *Application::applicationName = "Tundra";
const char *Application::version = "2.0.0";

Application::Application(Framework *owner, int &argc, char **argv) :
    QApplication(argc, argv),
    framework(owner),
    appActivated(true),
    nativeTranslator(new QTranslator),
    appTranslator(new QTranslator),
    targetFpsLimit(60.0)
#ifdef ENABLE_SPLASH_SCREEN
    ,splashScreen(0)
#endif
{
    // Reflect our versioning information to Qt internals, if something tries to obtain it straight from there.
    QApplication::setOrganizationName(organizationName);
    QApplication::setApplicationName(applicationName);
    QApplication::setApplicationVersion(version);

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

    if (!framework->Config()->HasValue(ConfigAPI::FILE_FRAMEWORK, ConfigAPI::SECTION_FRAMEWORK, "language"))
        framework->Config()->Set(ConfigAPI::FILE_FRAMEWORK, ConfigAPI::SECTION_FRAMEWORK, "language", "data/translations/tundra_en");

    QString default_language = framework->Config()->Get(ConfigAPI::FILE_FRAMEWORK, ConfigAPI::SECTION_FRAMEWORK, "language").toString();
    ChangeLanguage(default_language);

    QWebSettings::globalSettings()->setAttribute(QWebSettings::PluginsEnabled, true); //enable flash

    InitializeSplash();

    ReadTargetFpsLimitFromConfig();
}

Application::~Application()
{
#ifdef ENABLE_SPLASH_SCREEN
    SAFE_DELETE(splashScreen);
#endif
    SAFE_DELETE(nativeTranslator);
    SAFE_DELETE(appTranslator);
}

void Application::InitializeSplash()
{
#ifdef ENABLE_SPLASH_SCREEN
    if (framework->IsHeadless())
        return;

    if (!splashScreen)
    {
        QString runDir = InstallationDirectory();
        splashScreen = new QSplashScreen(QPixmap(runDir + "/data/ui/images/realxtend_tundra_splash.png"));
        splashScreen->setFont(QFont("Calibri", 9));
        splashScreen->show();
        SetSplashMessage("Initializing framework...");
    }
#endif
}

void Application::SetSplashMessage(const QString &message)
{
#ifdef ENABLE_SPLASH_SCREEN
    if (framework->IsHeadless())
        return;

    if (splashScreen && splashScreen->isVisible())
    {
        // As splash can go behind other widgets (so it does not obstruct startup debugging)
        // Make it show when a new message is set to it. This should keep it on top for the startup time,
        // but allow you to make it go to the background if you focuse something in front of it.
        splashScreen->activateWindow();

        // Call QApplication::processEvents() to update splash painting as at this point main loop is not running yet
        QString finalMessage = "v" + framework->ApplicationVersion()->GetVersion() + " - " + message.toUpper();
        splashScreen->showMessage(finalMessage, Qt::AlignBottom|Qt::AlignLeft, QColor(240, 240, 240));
        processEvents();
    }
#endif
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
#ifdef ENABLE_SPLASH_SCREEN
    SAFE_DELETE(splashScreen);
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
    std::cerr << "Application::Message not implemented for current platform!" << std::endl;
    assert(false && "Not implemented!");
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
    std::cerr << "Application::Message not implemented for current platform!" << std::endl;
    assert(false && "Not implemented!");
#endif
}

void Application::Message(const std::wstring &title, const std::wstring &text)
{
    Message(title.c_str(), text.c_str());
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
#else
    ///\todo Implement.
    LogWarning("Application::InstallationDirectory not implemented for this platform.");
    return ".";
#endif
}

QString Application::UserDataDirectory()
{
#ifdef _WINDOWS
    LPITEMIDLIST pidl;

    if (SHGetFolderLocation(0, CSIDL_APPDATA | CSIDL_FLAG_CREATE, NULL, 0, &pidl) != S_OK)
        return "";

    WCHAR str[MAX_PATH+1] = {};
    SHGetPathFromIDListW(pidl, str);
    CoTaskMemFree(pidl);

    return WStringToQString(str) + "\\" + ApplicationName();
#else
    ///\todo Convert to QString instead of std::string.
    char *ppath = 0;
    ppath = getenv("HOME");
    if (ppath == 0)
        throw Exception("Failed to get HOME environment variable.");

    std::string path(ppath);
    return QString((path + "/." + ApplicationName().toStdString()).c_str());
#endif
}

QString Application::UserDocumentsDirectory()
{
#ifdef _WINDOWS
    LPITEMIDLIST pidl;

    if (SHGetFolderLocation(0, CSIDL_PERSONAL | CSIDL_FLAG_CREATE, NULL, 0, &pidl) != S_OK)
        return "";

    WCHAR str[MAX_PATH+1] = {};
    SHGetPathFromIDListW(pidl, str);
    CoTaskMemFree(pidl);

    return WStringToQString(str) + '\\' + ApplicationName();
#else
    ///\todo Review. Is this desirable?
    return UserDataDirectory();
#endif
}

QString Application::ParseWildCardFilename(const QString& input)
{
    // Parse all the special symbols from the log filename.
    QString filename = input.trimmed().replace("$(CWD)", CurrentWorkingDirectory(), Qt::CaseInsensitive);
    filename = filename.replace("$(INSTDIR)", InstallationDirectory(), Qt::CaseInsensitive);
    filename = filename.replace("$(USERDATA)", UserDataDirectory(), Qt::CaseInsensitive);
    filename = filename.replace("$(USERDOCS)", UserDocumentsDirectory(), Qt::CaseInsensitive);
    QRegExp rx("\\$\\(DATE:(.*)\\)");
    // Qt Regexes don't support non-greedy matching. The above regex should be "\\$\\(DATE:(.*?)\\)". Instad Qt supports
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

void Application::ReadTargetFpsLimitFromConfig()
{
    ConfigData targetFpsConfigData(ConfigAPI::FILE_FRAMEWORK, ConfigAPI::SECTION_RENDERING);
    if (framework->Config()->HasValue(targetFpsConfigData, "fps target limit"))
    {
        bool ok;
        double targetFps = framework->Config()->Get(targetFpsConfigData, "fps target limit").toDouble(&ok);
        assert(ok && targetFps >= 0.0);
        if (ok && targetFps >= 0.0)
        {
            LogDebug("Application: read target FPS limit " + QString::number(targetFpsLimit) + " from config.");
            SetTargetFpsLimit(targetFps);
        }
        else
            LogWarning("Application: Invalid target FPS value " + QString::number(targetFps) + " read from config. Ignoring.");
    }
}

bool Application::eventFilter(QObject *obj, QEvent *event)
{
#ifdef Q_WS_MAC // workaround for Mac, because mouse events are not received as it ought to be
    QMouseEvent *mouse = dynamic_cast<QMouseEvent*>(event);
    if (mouse)
    {
        if (dynamic_cast<UiMainWindow*>(obj))
        {
            switch(event->type())
            {
            case QEvent::MouseButtonPress:
                framework->Ui()->GraphicsView()->mousePressEvent(mouse);
                break;
            case QEvent::MouseButtonRelease:
                framework->Ui()->GraphicsView()->mouseReleaseEvent(mouse);
                break;
            case QEvent::MouseButtonDblClick:
                framework->Ui()->GraphicsView()->mouseDoubleClickEvent(mouse);
                break;
            case QEvent::MouseMove:
                if (mouse->buttons() == Qt::LeftButton)
                    framework->Ui()->GraphicsView()->mouseMoveEvent(mouse);
                break;
            }
        }
    }
#endif
    try
    {
        if (obj == this)
        {
            if (event->type() == QEvent::ApplicationActivate)
                appActivated = true;
            if (event->type() == QEvent::ApplicationDeactivate)
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

bool Application::notify(QObject *receiver, QEvent *event)
{
    try
    {
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

        ///\note Ideally we should sleep 0 msecs when running at a high fps rate,
        /// but need to avoid QTimer::start() with 0 msecs, since that will cause the timer to immediately fire,
        /// which can cause the Win32 message loop inside Qt to starve. (Qt keeps spinning the timer.start(0) loop for Tundra mainloop and neglects Win32 API).
        double msecsToSleep = std::min(std::max(1.0, msecsPerFrame - msecsSpentInFrame), msecsPerFrame);

        // Reduce frame rate when unfocused
        if (!frameUpdateTimer.isActive())
        {
            if (appActivated || framework->IsHeadless())
                frameUpdateTimer.start((int)msecsToSleep);
            else
                frameUpdateTimer.start((int)(msecsToSleep + msecsPerFrame)); // Proceed at half FPS speed when unfocused (but never at half FPS when running a headless server).
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

void Application::AboutToExit()
{
    emit ExitRequested();
    // If no-one canceled the exit as a response to the signal, exit
    if (framework->IsExiting())
        quit();
}

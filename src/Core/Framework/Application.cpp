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

#include <boost/filesystem.hpp>
#include <iostream>
#include <utility>

#include <QDir>
#include <QGraphicsView>
#include <QTranslator>
#include <QLocale>
#include <QIcon>
#include <QWebSettings>
#include <QSplashScreen>

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

Application::Application(Framework *framework_, int &argc, char **argv) :
    QApplication(argc, argv),
    framework(framework_),
    appActivated(true),
    nativeTranslator(new QTranslator),
    appTranslator(new QTranslator),
    splashScreen(0)
{
    QApplication::setApplicationName("Tundra");

    // Make sure that the required Tundra data directories exist.
    boost::filesystem::wpath path(QStringToWString(UserDataDirectory()));
    if (!boost::filesystem::exists(path))
        boost::filesystem::create_directory(path);

    path = boost::filesystem::wpath(QStringToWString(UserDocumentsDirectory()));
    if (!boost::filesystem::exists(path))
        boost::filesystem::create_directory(path);

    // Add <install_dir>/qtplugins for qt to search plugins
    QString runDirectory = InstallationDirectory() + "/qtplugins";
    runDirectory.replace('\\', '/');
    addLibraryPath(runDirectory);

    QDir dir("data/translations/qt_native_translations");
    QStringList qmFiles = GetQmFiles(dir);

    // Search then that is there corresponding native translations for system locals.
    QString loc = QLocale::system().name();
    loc.chop(3);

    QString name = "data/translations/qt_native_translations/qt_" + loc + ".qm";
    QStringList lst = qmFiles.filter(name);
    if (!lst.empty() )
        nativeTranslator->load(lst[0]);

    this->installTranslator(nativeTranslator);

    if (!framework_->Config()->HasValue(ConfigAPI::FILE_FRAMEWORK, ConfigAPI::SECTION_FRAMEWORK, "language"))
        framework_->Config()->Set(ConfigAPI::FILE_FRAMEWORK, ConfigAPI::SECTION_FRAMEWORK, "language", "data/translations/naali_en");

    QString default_language = framework_->Config()->Get(ConfigAPI::FILE_FRAMEWORK, ConfigAPI::SECTION_FRAMEWORK, "language").toString();
    ChangeLanguage(default_language);

    QWebSettings::globalSettings()->setAttribute(QWebSettings::PluginsEnabled, true); //enablig flash

    InitializeSplash();
}

Application::~Application()
{
    SAFE_DELETE(splashScreen);
    SAFE_DELETE(nativeTranslator);
    SAFE_DELETE(appTranslator);
}

void Application::InitializeSplash()
{
// Don't show splash screen in debug mode as it 
// can obstruct your view if debugging the startup routines.
#ifndef _DEBUG

    if (framework->IsHeadless())
        return;

    if (!splashScreen)
    {
        QString runDir = InstallationDirectory();
#ifdef Q_WS_X11
        splashScreen = new QSplashScreen(QPixmap(runDir + "/data/ui/images/realxtend_tundra_splash.png"), Qt::WindowStaysOnTopHint|Qt::X11BypassWindowManagerHint);
#else
        splashScreen = new QSplashScreen(QPixmap(runDir + "/data/ui/images/realxtend_tundra_splash.png"), Qt::WindowStaysOnTopHint);
#endif
        splashScreen->setFont(QFont("Calibri", 9));
        splashScreen->show();
        SetSplashMessage("Initializing framework...");
    }
#endif
}

void Application::SetSplashMessage(const QString &message)
{
    if (framework->IsHeadless())
        return;

    if (splashScreen && splashScreen->isVisible())
    {
        // Call QApplication::processEvents() to update splash painting as at this point main loop is not running yet
        QString finalMessage = "v" + framework->ApplicationVersion()->GetVersion() + " - " + message.toUpper();
        splashScreen->showMessage(finalMessage, Qt::AlignBottom|Qt::AlignLeft, QColor(240, 240, 240));
        processEvents();
    }
}

QStringList Application::GetQmFiles(const QDir& dir)
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
    if (splashScreen)
    {
        splashScreen->close();
        SAFE_DELETE(splashScreen);
    }

    installEventFilter(this);

    QObject::connect(&frameUpdateTimer, SIGNAL(timeout()), this, SLOT(UpdateFrame()));
    frameUpdateTimer.setSingleShot(true);
    frameUpdateTimer.start(0);

    try
    {
        exec();
    }
    catch(const std::exception &e)
    {
        LogCritical(std::string("Application::Go caught an exception: ") + (e.what() ? e.what() : "(null)"));
        throw;
    }
    catch(...)
    {
        LogCritical(std::string("Application::Go caught an unknown exception!"));
        throw;
    }
}

void Application::Message(const std::string &title, const std::string &text)
{
#ifdef WIN32
    MessageBoxA(0, text.c_str(), title.c_str(), MB_OK | MB_ICONERROR | MB_TASKMODAL);
#else
    std::cerr << "Application::Message not implemented for current platform!" << std::endl;
    assert(false && "Not implemented!");
#endif
}

void Application::Message(const std::wstring &title, const std::wstring &text)
{
#ifdef WIN32
    MessageBoxW(0, text.c_str(), title.c_str(), MB_OK | MB_ICONERROR | MB_TASKMODAL);
#else
    std::cerr << "Application::Message not implemented for current platform!" << std::endl;
    assert(false && "Not implemented!");
#endif
}

void Application::SetCurrentWorkingDirectory(QString newCwd)
{
    ///\todo UNICODE support!
    boost::filesystem::current_path(newCwd.toStdString());
}

QString Application::CurrentWorkingDirectory()
{
#ifdef _WINDOWS
    WCHAR str[MAX_PATH+1] = {};
    GetCurrentDirectoryW(MAX_PATH, str);
    QString qstr = WStringToQString(str);
    if (!qstr.endsWith('\\'))
        qstr += '\\';
#else
    QString qstr =  QDir::currentPath();
    if (!qstr.endsWith('/'))
        qstr += '/';
#endif

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
    return ".";
#endif
}

QString Application::UserDataDirectory()
{
    const QString applicationName = "Tundra"; ///\todo Move applicationName from Config API to the Application class. Make it static without a runtime setter.
#ifdef _WINDOWS
    LPITEMIDLIST pidl;

    if (SHGetFolderLocation(0, CSIDL_APPDATA | CSIDL_FLAG_CREATE, NULL, 0, &pidl) != S_OK)
        return "";

    WCHAR str[MAX_PATH+1] = {};
    SHGetPathFromIDListW(pidl, str);
    CoTaskMemFree(pidl);

    return WStringToQString(str) + "\\" + applicationName;
#else
    ///\todo Convert to QString instead of std::string.
    char *ppath = 0;
    ppath = getenv("HOME");
    if (ppath == 0)
        throw Exception("Failed to get HOME environment variable.");

    std::string path(ppath);
    return QString((path + "/." + applicationName.toStdString()).c_str());
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

    const QString applicationName = "Tundra"; ///\todo Move applicationName from Config API to the Application class. Make it static without a runtime setter.
    return WStringToQString(str) + '\\' + applicationName;
#else
    ///\todo Review. Is this desirable?
    return UserDataDirectory();
#endif
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
        std::cout << std::string("QApp::eventFilter caught an exception: ") + (e.what() ? e.what() : "(null)") << std::endl;
        LogCritical(std::string("QApp::eventFilter caught an exception: ") + (e.what() ? e.what() : "(null)"));
        throw;
    } catch(...)
    {
        std::cout << std::string("QApp::eventFilter caught an unknown exception!") << std::endl;
        LogCritical(std::string("QApp::eventFilter caught an unknown exception!"));
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
    } catch(const std::exception &e)
    {
        std::cout << std::string("QApp::notify caught an exception: ") << (e.what() ? e.what() : "(null)") << std::endl;
        LogCritical(std::string("QApp::notify caught an exception: ") + (e.what() ? e.what() : "(null)"));
        throw;
    } catch(...)
    {
        std::cout << std::string("QApp::notify caught an unknown exception!") << std::endl;
        LogCritical(std::string("QApp::notify caught an unknown exception!"));
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

        const boost::program_options::variables_map &options = framework->ProgramOptions();
        double targetFpsLimit = 60.0;
        if (options.count("fpslimit") > 0)
        {
            targetFpsLimit = options["fpslimit"].as<float>();
            if (targetFpsLimit < 1.f)
                targetFpsLimit = 0.f;
        }

        tick_t timeNow = GetCurrentClockTime();

        static tick_t timerFrequency = GetCurrentClockFreq();

        double msecsSpentInFrame = (double)(timeNow - frameStartTime) * 1000.0 / timerFrequency;
        const double msecsPerFrame = 1000.0 / targetFpsLimit;

        double msecsToSleep = std::min(std::max(0.0, msecsPerFrame - msecsSpentInFrame), msecsPerFrame);

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
        std::cout << "QApp::UpdateFrame caught an exception: " << (e.what() ? e.what() : "(null)") << std::endl;
        LogCritical(std::string("QApp::UpdateFrame caught an exception: ") + (e.what() ? e.what() : "(null)"));
        throw;
    }
    catch(...)
    {
        std::cout << "QApp::UpdateFrame caught an unknown exception!" << std::endl;
        LogCritical(std::string("QApp::UpdateFrame caught an unknown exception!"));
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

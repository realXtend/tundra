// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "Application.h"
#include "Framework.h"
#include "Profiler.h"
#include "CoreStringUtils.h"
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
    appTranslator(new QTranslator)
{
    QApplication::setApplicationName("Tundra");

    // Make sure that the required Tundra data directories exist.
    boost::filesystem::wpath path(QStringToWString(UserDataDirectory()));
    if (!boost::filesystem::exists(path))
        boost::filesystem::create_directory(path);

    path = boost::filesystem::wpath(QStringToWString(UserDocumentsDirectory()));
    if (!boost::filesystem::exists(path))
        boost::filesystem::create_directory(path);

#if defined(Q_WS_WIN) || defined(Q_WS_MAC)
        // If under windows, add run_dir/plugins as library path
        // unix users will get plugins from their OS Qt installation folder automatically

        QString runDirectory = InstallationDirectory() + "/qtplugins";
        runDirectory.replace('\\', '/');
        addLibraryPath(runDirectory);
#endif

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
/*    ///\todo Reimplement.
    std::string default_language = framework->GetConfigManager()->DeclareSetting(Framework::ConfigurationGroup(),
        "language", std::string("data/translations/naali_en"));*/
    ChangeLanguage("data/translations/naali_en");

    QWebSettings::globalSettings()->setAttribute(QWebSettings::PluginsEnabled, true); //enablig flash
}

Application::~Application()
{
    SAFE_DELETE(nativeTranslator);
    SAFE_DELETE(appTranslator);
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
    installEventFilter(this);

    QObject::connect(&frameUpdateTimer, SIGNAL(timeout()), this, SLOT(UpdateFrame()));
    frameUpdateTimer.setSingleShot(true);
    frameUpdateTimer.start(0); 

    try
    {
        exec ();
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
    MessageBoxA(0, text.c_str(), title.c_str(), MB_OK | MB_ICONERROR | MB_TASKMODAL);
}

void Application::Message(const std::wstring &title, const std::wstring &text)
{
    MessageBoxW(0, text.c_str(), title.c_str(), MB_OK | MB_ICONERROR | MB_TASKMODAL);
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
#ifdef _WINDOWS
    LPITEMIDLIST pidl;

    if (SHGetFolderLocation(0, CSIDL_APPDATA | CSIDL_FLAG_CREATE, NULL, 0, &pidl) != S_OK)
        return "";

    WCHAR str[MAX_PATH+1] = {};
    SHGetPathFromIDListW(pidl, str);
    CoTaskMemFree(pidl);

    const QString applicationName = "Tundra"; ///\todo Move applicationName from Config API to the Application class. Make it static without a runtime setter.
    return WStringToQString(str) + "\\" + applicationName;
#else
    ///\todo Convert to QString instead of std::string.
    char *ppath = 0;
    ppath = getenv("HOME");
    if (ppath == 0)
        throw Exception("Failed to get HOME environment variable.");

    std::string path(ppath);
    return path + "/." + std::string(framework_->Config()->GetApplicationName().toStdString());
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
///\todo Reimplement.
//        framework->GetConfigManager()->SetSetting(Framework::ConfigurationGroup(), "language", file.toStdString());
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

// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "Application.h"
#include "Framework.h"
#include "VersionInfo.h"
#include "ConfigurationManager.h"
#include "CoreStringUtils.h"

#include "InputAPI.h"
#include "UiAPI.h"
#include "UiMainWindow.h"

#include <QDir>
#include <QGraphicsView>
#include <QTranslator>
#include <QLocale>
#include <QIcon>
#include <QWebSettings>
#include <QSplashScreen>
#include <QFont>

#include <QDebug>

#ifdef Q_WS_MAC
#include <QMouseEvent>
#include <QWheelEvent>
#include "UiGraphicsView.h"
#endif

#include "MemoryLeakCheck.h"

using namespace Foundation;

Application::Application(Framework *framework_, int &argc, char **argv) :
    QApplication(argc, argv),
    framework(framework_),
    nativeTranslator(new QTranslator),
    appTranslator(new QTranslator),
    splashScreen(0),
    targetFps_(0.0f),
    fwSkipFrames_(0)
{
    QApplication::setApplicationName("realXtend-Tundra");

#if defined(Q_WS_WIN) || defined(Q_WS_MAC)
    // If under windows, add run_dir/plugins as library path
    // unix users will get plugins from their OS Qt installation folder automatically
    QString runDirectory = QString::fromStdString(ReplaceChar(framework->GetPlatform()->GetInstallDirectory(), '\\', '/'));
    runDirectory += "/qtplugins";
    addLibraryPath(runDirectory);
#endif

    // Initialize languages
    // - Search then that is there corresponding native translations for system locals.
    QDir dir("data/translations/qt_native_translations");
    QStringList qmFiles = GetQmFiles(dir);
    QString loc = QLocale::system().name();
    loc.chop(3);

    QString name = "data/translations/qt_native_translations/qt_" + loc + ".qm";
    QStringList lst = qmFiles.filter(name);
    if (!lst.empty() )
        nativeTranslator->load(lst[0]);
    this->installTranslator(nativeTranslator);
    
    std::string default_language = framework->GetConfigManager()->DeclareSetting(Framework::ConfigurationGroup(),
        "language", std::string("data/translations/naali_en"));
    ChangeLanguage(QString::fromStdString(default_language));

    // Enable plugins for global web settings, eg. Flash for the browser.
    QWebSettings::globalSettings()->setAttribute(QWebSettings::PluginsEnabled, true);

    // Framework update fps limit from start params
    const boost::program_options::variables_map &options = framework_->ProgramOptions();
    if (options.count("fpslimit") > 0)
    {
        targetFps_ = options["fpslimit"].as<float>();
        if (targetFps_ < 1.f)
            targetFps_ = 0.f;
    }
    else
        targetFps_ = 60.f;
    targetFpsStartParam_ = targetFps_;
    timerFrequency_ = GetCurrentClockFreq();
}

Application::~Application()
{
    SAFE_DELETE(splashScreen);
    SAFE_DELETE(nativeTranslator);
    SAFE_DELETE(appTranslator);
}

void Application::InitializeSplash()
{
    if (framework->IsHeadless())
        return;

    if (!splashScreen)
    {
#ifdef Q_WS_X11
        splashScreen = new QSplashScreen(QPixmap("./data/ui/images/splash.png"), Qt::WindowStaysOnTopHint|Qt::X11BypassWindowManagerHint);
#else
        splashScreen = new QSplashScreen(QPixmap("./data/ui/images/splash.png"), Qt::WindowStaysOnTopHint);
#endif
        splashScreen->setFont(QFont("Calibri", 9));
        splashScreen->show();
        SetSplashMessage("Initializing framework...");
    }
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
    // Close splash screen and show main window
    if (splashScreen)
        splashScreen->close();
    if (!framework->IsHeadless() && framework->Ui()->MainWindow())
        framework->Ui()->MainWindow()->show();

    // Install QObject event filter for QApplication, every thing runs trough this filter
    installEventFilter(this);

    try
    {
        fwUpdateId_ = startTimer(1);
        exec();
    }
    catch(const std::exception &e)
    {
        RootLogCritical(std::string("NaaliApplication::Go() Caught an exception: ") + (e.what() ? e.what() : "(null)"));
        throw;
    }
    catch(...)
    {
        RootLogCritical(std::string("NaaliApplication::Go() Caught an unknown exception!"));
        throw;
    }
}

bool Application::eventFilter(QObject *obj, QEvent *event)
{
    // Workaround for Mac, because mouse events are not behaving as expected 
#ifdef Q_WS_MAC 
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
        if (framework && !framework->IsHeadless())
        {
            if (obj == framework->Ui()->MainWindow() && event->type() == QEvent::Move)
                fwSkipFrames_ = 5;
        }

        if (obj == this)
        {
            // Drop down fps on a inactive app
            if (event->type() == QEvent::ApplicationActivate)
                RestoreTargetFps();
            if (event->type() == QEvent::ApplicationDeactivate)
                SetTargetFps(30.0f); 
        }

        return QObject::eventFilter(obj, event);
    }
    catch(const std::exception &e)
    {
        std::cout << std::string("QApp::eventFilter caught an exception: ") + (e.what() ? e.what() : "(null)") << std::endl;
        RootLogCritical(std::string("QApp::eventFilter caught an exception: ") + (e.what() ? e.what() : "(null)"));
        throw;
    } catch(...)
    {
        std::cout << std::string("QApp::eventFilter caught an unknown exception!") << std::endl;
        RootLogCritical(std::string("QApp::eventFilter caught an unknown exception!"));
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
        framework->GetConfigManager()->SetSetting(Framework::ConfigurationGroup(), "language", file.toStdString());
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
        std::cout << std::string("QApp::notify caught an exception: ") << (e.what() ? e.what() : "(null)") << std::endl;
        RootLogCritical(std::string("QApp::notify caught an exception: ") + (e.what() ? e.what() : "(null)"));
        throw;
    } 
    catch(...)
    {
        std::cout << std::string("QApp::notify caught an unknown exception!") << std::endl;
        RootLogCritical(std::string("QApp::notify caught an unknown exception!"));
        throw;
    }
}

void Application::timerEvent(QTimerEvent *event)
{
    if (fwUpdateId_ == event->timerId())
        UpdateFrame();
}

void Application::UpdateFrame()
{
    // Don't pump the QEvents to QApplication if we are exiting
    // also don't process our main loop frames.
    if (framework->IsExiting())
        return;

    // No target fps limiting for time critical APIs
    framework->UpdateTimeCriticalAPIs();

    // Check fps limiter if we should update our modules
    if (RunFrameworkUpdate())
    {
        // Mark last time before doing updates, so we don't start to lag behind the target fps
        lastPresentTime_ = GetCurrentClockTime();
        
        try
        {
            PROFILE(Update_MainLoop);

            double frametime = framework->CalculateFrametime();
            framework->UpdateModules(frametime);
            framework->UpdateAPIs(frametime);
            framework->UpdateRendering(frametime);
            
            // Disabled the rendering skipping when window is moved for now...
            // windows specific freeze when main window is moved from monitor to monitor
            //if (fwSkipFrames_ == 0)
            //    framework->UpdateRendering(frametime);
            //else
            //    fwSkipFrames_--;

            RESETPROFILER
        }
        catch(const std::exception &e)
        {
            std::cout << "QApp::UpdateFrame caught an exception: " << (e.what() ? e.what() : "(null)") << std::endl;
            RootLogCritical(std::string("QApp::UpdateFrame caught an exception: ") + (e.what() ? e.what() : "(null)"));
            throw;
        }
        catch(...)
        {
            std::cout << "QApp::UpdateFrame caught an unknown exception!" << std::endl;
            RootLogCritical(std::string("QApp::UpdateFrame caught an unknown exception!"));
            throw;
        }
    }
}

bool Application::RunFrameworkUpdate()
{
    if (targetFps_ >= 1.0f)
    {
        tick_t timeNow = GetCurrentClockTime();
        double currentFrameTime = (double)(timeNow - lastPresentTime_) * 1000.0 / timerFrequency_;
        const double msecsPerFrame = 1000.0 / targetFps_;
        if (currentFrameTime < msecsPerFrame)
            return false;
    }
    return true;
}

void Application::SetTargetFps(float fps)
{
    if (fps < 1.0f)
        fps = 0.0f;
    targetFps_ = fps;
}

void Application::RestoreTargetFps()
{
    targetFps_ = targetFpsStartParam_;
}

void Application::AboutToExit()
{
    emit ExitRequested();
    
    // If no-one canceled the exit as a response to the signal, exit
    if (framework->IsExiting())
        quit();
}

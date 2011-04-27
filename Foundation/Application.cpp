// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "Application.h"
#include "Framework.h"
#include "ConfigurationManager.h"
#include "CoreStringUtils.h"

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

#include "MemoryLeakCheck.h"

using namespace Foundation;

Application::Application(Framework *framework_, int &argc, char **argv) :
    QApplication(argc, argv),
    framework(framework_),
    appActivated(true),
    nativeTranslator(new QTranslator),
    appTranslator(new QTranslator)
{
    QApplication::setApplicationName("realXtend-Naali");

#if defined(Q_WS_WIN) || defined(Q_WS_MAC)
        // If under windows, add run_dir/plugins as library path
        // unix users will get plugins from their OS Qt installation folder automatically

        QString runDirectory = QString::fromStdString(ReplaceChar(framework->GetPlatform()->GetInstallDirectory(), '\\', '/'));
        runDirectory += "/qtplugins";
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
    
    std::string default_language = framework->GetConfigManager()->DeclareSetting(Framework::ConfigurationGroup(),
        "language", std::string("data/translations/naali_en"));
    ChangeLanguage(QString::fromStdString(default_language));

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
        RootLogCritical(std::string("NaaliApplication::Go caught an exception: ") + (e.what() ? e.what() : "(null)"));
        throw;
    }
    catch(...)
    {
        RootLogCritical(std::string("NaaliApplication::Go caught an unknown exception!"));
        throw;
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
    } catch(const std::exception &e)
    {
        std::cout << std::string("QApp::notify caught an exception: ") << (e.what() ? e.what() : "(null)") << std::endl;
        RootLogCritical(std::string("QApp::notify caught an exception: ") + (e.what() ? e.what() : "(null)"));
        throw;
    } catch(...)
    {
        std::cout << std::string("QApp::notify caught an unknown exception!") << std::endl;
        RootLogCritical(std::string("QApp::notify caught an unknown exception!"));
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
                frameUpdateTimer.start(msecsToSleep); 
            else 
                frameUpdateTimer.start(msecsToSleep + msecsPerFrame); // Proceed at half FPS speed when unfocused (but never at half FPS when running a headless server).
        }
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

void Application::AboutToExit()
{
    emit ExitRequested();
    
    // If no-one canceled the exit as a response to the signal, exit
    if (framework->IsExiting())
        quit();
}

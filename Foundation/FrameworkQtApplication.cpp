// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "FrameworkQtApplication.h"
#include "Framework.h"
#include "ConfigurationManager.h"

#include <QDir>
#include <QGraphicsView>
#include <QTranslator>
#include <QLocale>

#include "MemoryLeakCheck.h"

namespace Foundation
{
    FrameworkQtApplication::FrameworkQtApplication(Framework *framework, int &argc, char **argv) :
        QApplication(argc, argv), framework_(framework), app_activated_(true), native_translator_(new QTranslator), app_translator_(new QTranslator)
    {
#ifdef Q_WS_WIN
        // If under windows, add run_dir/plugins as library path
        // unix users will get plugins from their OS Qt installation folder automatically
        QString run_directory = applicationDirPath();
        run_directory += "/qtplugins";
        addLibraryPath(run_directory);
#endif

        QDir dir("data/translations/qt_native_translations");
        QStringList qmFiles = GetQmFiles(dir);

        // Search then that is there corresponding native translations for system locals.
        QString loc = QLocale::system().name();
        loc.chop(3);

        QString name = "data/translations/qt_native_translations/qt_" + loc + ".qm";
        QStringList lst = qmFiles.filter(name);
        if (!lst.empty() )
            native_translator_->load(lst[0]);

        this->installTranslator(native_translator_);
        
        std::string default_language = framework_->GetConfigManager()->DeclareSetting(Framework::ConfigurationGroup(), "language", std::string("data/translations/naali_en"));
        ChangeLanguage(QString::fromStdString(default_language));
    }

    FrameworkQtApplication::~FrameworkQtApplication()
    {
        SAFE_DELETE(native_translator_);
        SAFE_DELETE(app_translator_);
    }

    QGraphicsView *FrameworkQtApplication::GetUIView() const
    {
        return view_.get();
    }

    void FrameworkQtApplication::SetUIView(std::auto_ptr <QGraphicsView> view)
    {
        view_ = view;
    }

    QStringList FrameworkQtApplication::GetQmFiles(const QDir& dir)
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

    void FrameworkQtApplication::Go()
    {
        installEventFilter(this);

        QObject::connect(&frame_update_timer_, SIGNAL(timeout()), this, SLOT(UpdateFrame()));
        frame_update_timer_.setSingleShot (true);
        frame_update_timer_.start (0); 

        try
        {
            exec ();
        }
        catch(const std::exception &e)
        {
            RootLogCritical(std::string("FrameworkQtApplication::Go caught an exception: ") + (e.what() ? e.what() : "(null)"));
            throw;
        }
        catch(...)
        {
            RootLogCritical(std::string("FrameworkQtApplication::Go caught an unknown exception!"));
            throw;
        }
    }
    
    bool FrameworkQtApplication::eventFilter(QObject *obj,  QEvent *event)
    {
        try
        {
            if (obj == this)
            {
                if (event->type() == QEvent::ApplicationActivate)
                    app_activated_ = true;
                if (event->type() == QEvent::ApplicationDeactivate)
                    app_activated_ = false;
                /*
                if (event->type() == QEvent::LanguageChange)
                {
                    // Now if exist remove our default translation engine.
                    if (translator_ != 0)
                        this->removeTranslator(translator_);
                }
                */
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
        return true;
    }

    void FrameworkQtApplication::ChangeLanguage(const QString& file)
    {
        QString filename = file;
        if (!filename.endsWith(".qm", Qt::CaseInsensitive))
            filename.append(".qm");
        QString tmp = filename;
        tmp.chop(3);
        QString str = tmp.right(2);
        
        QString name = "data/translations/qt_native_translations/qt_" + str + ".qm";

        // Remove old translators then change them to new. 
        removeTranslator(native_translator_); 

        if ( QFile::exists(name) )
        {
            if ( native_translator_ != 0)
            {
                native_translator_->load(name);
                installTranslator(native_translator_); 
            }
        }
        else
        {
            if (native_translator_ != 0 && native_translator_->isEmpty())
            {
                installTranslator(native_translator_);
            }
            else
            {
                SAFE_DELETE(native_translator_);
                native_translator_ = new QTranslator;
                installTranslator(native_translator_); 
            }
        }

        // Remove old translators then change them to new. 
        removeTranslator(app_translator_);
        if (app_translator_->load(filename))
        {
            installTranslator(app_translator_);
            framework_->GetConfigManager()->SetSetting(Framework::ConfigurationGroup(), "language", file.toStdString());
        }
        
        emit LanguageChanged();
    }

    bool FrameworkQtApplication::notify(QObject *receiver, QEvent *event)
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
        return true;
    }
    
    void FrameworkQtApplication::UpdateFrame()
    {
        try
        {
            QApplication::processEvents (QEventLoop::AllEvents, 1);
            QApplication::sendPostedEvents ();

            framework_-> ProcessOneFrame();

            // Reduce framerate when unfocused
            if (app_activated_)
                frame_update_timer_.start(0); 
            else 
                frame_update_timer_.start(5);
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
    
    void FrameworkQtApplication::AboutToExit()
    {
        emit ExitRequested();
        
        // If no-one canceled the exit as a response to the signal, exit
        if (framework_->IsExiting())
            quit();
    }
}

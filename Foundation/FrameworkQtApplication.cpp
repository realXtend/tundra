// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"

#include "Framework.h"
#include "FrameworkQtApplication.h"

namespace Foundation
{

    FrameworkQtApplication::FrameworkQtApplication (Framework *framework, int &argc, char** argv) : 
        QApplication (argc, argv), framework_ (framework), app_activated_(true)
    {
        #ifdef Q_WS_WIN
        // If under windows, add run_dir/plugins as library path
        // unix users will get plugins from their OS Qt installation folder automatically
        QString run_directory = applicationDirPath();
        run_directory += "/qtplugins";
        addLibraryPath(run_directory);
        #endif
    }

    FrameworkQtApplication::~FrameworkQtApplication()
    {
    }

    QGraphicsView *FrameworkQtApplication::GetUIView() const
    {
        return view_.get();
    }

    void FrameworkQtApplication::SetUIView(std::auto_ptr <QGraphicsView> view)
    {
        view_ = view;
    }

    void FrameworkQtApplication::Go()
    {
        installEventFilter(this);
        
        QObject::connect(&frame_update_timer_, SIGNAL(timeout()), this, SLOT(UpdateFrame()));
        frame_update_timer_.setSingleShot (true);
        frame_update_timer_.start (0); 

        exec ();
    }
    
    bool FrameworkQtApplication::eventFilter(QObject *obj,  QEvent *event)
    {
        if (obj == this)
        {
            if (event->type() == QEvent::ApplicationActivate)
                app_activated_ = true;
            if (event->type() == QEvent::ApplicationDeactivate)
                app_activated_ = false;
        }
        return QObject::eventFilter(obj, event);
    }

    void FrameworkQtApplication::UpdateFrame()
    {
        PROFILE(FrameworkQtApplication_UpdateFrame);

        QApplication::processEvents (QEventLoop::AllEvents, 1);
        QApplication::sendPostedEvents ();

        framework_-> ProcessOneFrame();

        // Reduce framerate when unfocused
        if (app_activated_)
            frame_update_timer_.start (0); 
        else 
            frame_update_timer_.start (5);
    }

}

// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"

#include "Framework.h"
#include "FrameworkQtApplication.h"

namespace Foundation
{

    FrameworkQtApplication::FrameworkQtApplication (Framework *framework, int &argc, char** argv) : 
        QApplication (argc, argv), framework_ (framework)
    {
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
        QObject::connect(&frame_update_timer_, SIGNAL(timeout()), this, SLOT(UpdateFrame()));

        // with single shot and start(0) processor usage = 90-98% no matter the size of window
        // with start(20) its <50% all the time
        frame_update_timer_.setSingleShot (true);
        frame_update_timer_.start (0); 

        exec ();
    }
    
    void FrameworkQtApplication::UpdateFrame()
    {
        QApplication::processEvents (QEventLoop::AllEvents, 1);
        QApplication::sendPostedEvents ();

        framework_-> ProcessOneFrame();

        frame_update_timer_.start (0); 
    }
}

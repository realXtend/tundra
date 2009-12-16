// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "QOgreUIView.h"

#include "Framework.h"
#include "QtFrameworkEngine.h"

namespace Foundation
{

    QtFrameworkEngine::QtFrameworkEngine(Framework *owner, int &argc, char** argv) : 
        owner_(owner)
    {
        app_.reset (new QApplication(argc, argv));
    }

    QtFrameworkEngine::~QtFrameworkEngine()
    {
        view_.reset(0);
        app_.reset(0);
    }

    QApplication *QtFrameworkEngine::GetQApplication() const
    {
        return app_.get();
    }

    QGraphicsView *QtFrameworkEngine::GetUIView() const
    {
        return view_.get();
    }

    void QtFrameworkEngine::SetUIView(std::auto_ptr <QGraphicsView> view)
    {
        view_ = view;
    }

    void QtFrameworkEngine::AddUIViewListener (QObject *listener)
    {
        if (view_.get()) view_-> installEventFilter (listener);
    }
    
    void QtFrameworkEngine::UpdateFrame()
    {
        assert(owner_);
        owner_->ProcessOneFrame();
        frame_update_timer_.start(0);
    }

    void QtFrameworkEngine::Go()
    {
        assert(app_.get());

        QObject::connect(app_.get(), SIGNAL(aboutToQuit()), this, SLOT(OnAboutToQuit()));
        QObject::connect(&frame_update_timer_, SIGNAL(timeout()), this, SLOT(UpdateFrame()));
        
        // with single shot and start(0) processor usage = 90-98% no matter the size of window
        // with start(20) its <50% all the time
        frame_update_timer_.setSingleShot(true);
        frame_update_timer_.start(0); 

        app_->exec();
    }

    void QtFrameworkEngine::OnAboutToQuit()
    {
        assert(owner_);
        if (owner_)
            owner_->Exit();
    }

    void QtFrameworkEngine::SendQAppQuitMessage()
    {
        assert(app_.get());
        if (app_.get())
            app_->quit();
    }
}

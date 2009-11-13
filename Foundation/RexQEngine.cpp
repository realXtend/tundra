// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"

#include <sstream>
#include <string>

#include <QApplication>
#include <QWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <QFrame>

#ifndef Q_WS_WIN
#include <QX11Info>
#endif

#include "Framework.h"

#include "RexQEngine.h"

namespace Foundation
{

RexQEngine::RexQEngine(Framework *owner, int &argc, char** argv)
:owner_(owner)
{
    app_ = new QApplication(argc, argv);

    // The following code creates a new QWidget as the main window to host the Ogre frame.
    // Not using this method currently due to composition problems. 
    // (render window & widget client area cannot safely overlap)
/*
    ogre_host_widget_ = new QWidget();

    int width = owner->GetDefaultConfig().DeclareSetting("OgreRenderer", "window_width", 800);
    int height = owner->GetDefaultConfig().DeclareSetting("OgreRenderer", "window_height", 600);
    ogre_host_widget_->resize(width, height);

    ogre_host_widget_->setWindowTitle("Ogre3D Window");
#ifndef Q_WS_WIN 
    // At the begin render window and widget should be same size.  
    ogre_host_widget_->resize(800,600);
 
#endif
    ogre_host_widget_->show();
*/
}

RexQEngine::~RexQEngine()
{
    delete app_;
    app_ = 0;
}

std::string RexQEngine::GetMainWindowHandle() const
{
//    assert(false);
//    return "";

  std::stringstream windowHandle;

#ifndef Q_WS_WIN
 
    QX11Info info = ogre_host_widget_->x11Info();
    windowHandle << (unsigned long)(info.display());
    windowHandle << ":";
    windowHandle << static_cast<unsigned int>(info.screen());
    windowHandle << ":";
  
  // This which one is used depends that is widget allready created and is show()-method called. 
  if (ogre_host_widget_->isHidden())
    windowHandle << static_cast<unsigned long>(ogre_host_widget_->effectiveWinId());
  else
    windowHandle << static_cast<unsigned long>(ogre_host_widget_->winId());

 #else
 
//    windowHandle << (size_t)(HWND)ogre_host_widget_->winId();
//    windowHandle << (size_t)(HWND)ogre_frame_->winId();
#endif
 
   return windowHandle.str();   

}

QWidget *RexQEngine::GetMainWindowQWidget() const
{
    assert(false);
    return 0;
//   return ogre_host_widget_;
}

QApplication *RexQEngine::GetQApplication() const
{
    return app_;
}

void RexQEngine::UpdateFrame()
{
    assert(owner_);

    owner_->ProcessOneFrame();

    /// \todo Compute a new delta time. \todo Break down update and render.
    frame_update_timer_.start(0);
}

void RexQEngine::Go()
{
    assert(app_);

    QObject::connect(app_, SIGNAL(aboutToQuit()), this, SLOT(OnAboutToQuit()));


    // Application frame updates are processed according to the periodic
    // ticks of this timer.
    QObject::connect(&frame_update_timer_, SIGNAL(timeout()), this, SLOT(UpdateFrame()));
    frame_update_timer_.setSingleShot(true);
    frame_update_timer_.start(0);

//    QObject::connect(app, SIGNAL(guiThreadAwake()), this, SLOT(UpdateFrame()));

    app_->exec();
}

void RexQEngine::OnAboutToQuit()
{
    assert(owner_);
    if (owner_)
        owner_->Exit();
}

void RexQEngine::SendQAppQuitMessage()
{
    assert(app_);
    if (app_)
        app_->quit();
}

}

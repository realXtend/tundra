#include "StableHeaders.h"

#include <sstream>
#include <string>

#include <QApplication>
#include <QWidget>
#include <QPushButton>
#include <QHBoxLayout>

#ifndef Q_WS_WIN
#include <QX11Info>
#endif

#include "Framework.h"

#include "RexQEngine.h"

namespace Foundation
{

RexQEngine::RexQEngine(Framework *owner)
:owner_(owner)
{
    int argc = 0;
    app_ = new QApplication(argc, 0);
    ogre_host_widget_ = new QWidget();

    ogre_host_widget_->setWindowTitle("Ogre3D Window");
#ifndef Q_WS_WIN 
    // At the begin render window and widget should be same size.  
    ogre_host_widget_->resize(800,600);
 
#endif
    ogre_host_widget_->show();
}

RexQEngine::~RexQEngine()
{
    delete app_;
    app_ = 0;
}

std::string RexQEngine::GetMainWindowHandle() const
{
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
 
    windowHandle << (size_t)(HWND)ogre_host_widget_->winId();
 
#endif
 
   return windowHandle.str();   

}

void RexQEngine::UpdateFrame()
{
    assert(owner_);

    owner_->ProcessOneFrame();

    /// \todo Compute a new delta time. \todo Break down update and render.
    frame_update_timer_.start(16);
}

void RexQEngine::Go()
{
    assert(app_);

    QObject::connect(app_, SIGNAL(aboutToQuit()), this, SLOT(OnQuitQApp()));


    // Application frame updates are processed according to the periodic
    // ticks of this timer.
    QObject::connect(&frame_update_timer_, SIGNAL(timeout()), this, SLOT(UpdateFrame()));
    frame_update_timer_.setSingleShot(true);
    frame_update_timer_.start(16);

    app_->exec();
}

void RexQEngine::OnQuitQApp()
{
    owner_->Exit();
    app_->quit();
}

}

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

RexQEngine::RexQEngine(Framework *owner)
:owner_(owner)
{
    int argc = 0;
    app_ = new QApplication(argc, 0);

//    ogre_host_widget_ = new QWidget();

    int width = owner->GetDefaultConfig().DeclareSetting("OgreRenderer", "window_width", 800);
    int height = owner->GetDefaultConfig().DeclareSetting("OgreRenderer", "window_height", 600);
//    ogre_host_widget_->resize(width, height);

/*
    ogre_host_widget_->setWindowTitle("Ogre3D Window");
#ifndef Q_WS_WIN 
    // At the begin render window and widget should be same size.  
    ogre_host_widget_->resize(800,600);
 
#endif
    ogre_host_widget_->show();

//    box_layout_ = new QHBoxLayout(ogre_host_widget_);
//    ogre_frame_ = new QWidget(ogre_host_widget_);
//    ogre_frame_ = new QWidget();

//    QWidget widget(ogre_host_widget_);
//    widget.setWindowTitle("Start window");
//    widget.show();
//    QHBoxLayout layOut; 
//    QPushButton* pButton = new QPushButton("&Start our awesome application", ogre_host_widget_);

//    layOut.addWidget(pButton);
//    widget.setLayout(&layOut);
//    box_layout_->addWidget(ogre_frame_);
//    box_layout_->addWidget(pButton);
//    ogre_host_widget_->setLayout(box_layout_);
    

//    ogre_frame_ = new QFrame(ogre_host_widget_);
//    ogre_frame_->
*/
}

RexQEngine::~RexQEngine()
{
    delete app_;
    app_ = 0;
}

std::string RexQEngine::GetMainWindowHandle() const
{
    assert(false);
    return "";
/*
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
//    windowHandle << (size_t)(HWND)ogre_frame_->winId();
#endif
 
   return windowHandle.str();   
*/
}

QWidget *RexQEngine::GetMainWindowQWidget() const
{
    assert(false);
    return 0;
//    return ogre_host_widget_;
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
  
   //    QObject::connect(pButton, SIGNAL(clicked()), this, SLOT(UpdateFrame()));

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

// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "ExternalRenderWindow.h"

#include <QResizeEvent>
#include <QKeyEvent>

#ifdef Q_WS_X11
#include <QX11Info>
#endif

#ifdef Q_WS_WIN
#include "Win.h"
#endif

namespace CAVEStereo
{
    ExternalRenderWindow::ExternalRenderWindow()
    {
    }
    ExternalRenderWindow::~ExternalRenderWindow()
    {
    }
    Ogre::RenderWindow* ExternalRenderWindow::CreateRenderWindow(const std::string &name,  int width, int height, int left, int top, bool fullscreen)
    {
        bool stealparent 
            ((parentWidget())? true : false);

        QWidget *nativewin 
            ((stealparent)? parentWidget() : this);

        Ogre::NameValuePairList params;
        Ogre::String winhandle;

#ifdef Q_WS_WIN
        // According to Ogre Docs
        // positive integer for W32 (HWND handle)
        winhandle = Ogre::StringConverter::toString 
            ((unsigned int) 
             (nativewin-> winId ()));

        //Add the external window handle parameters to the existing params set.
        params["externalWindowHandle"] = winhandle;

#endif

#ifdef Q_WS_MAC
    // qt docs say it's a HIViewRef on carbon,
    // carbon docs say HIViewGetWindow gets a WindowRef out of it

#if 0
    HIViewRef vref = (HIViewRef) nativewin-> winId ();
    WindowRef wref = HIViewGetWindow(vref);
        winhandle = Ogre::StringConverter::toString(
           (unsigned long) (HIViewGetRoot(wref)));
#else
        // according to
        // http://www.ogre3d.org/forums/viewtopic.php?f=2&t=27027 does
        winhandle = Ogre::StringConverter::toString(
                     (unsigned long) nativewin->winId());
#endif
        //Add the external window handle parameters to the existing params set.
        params["externalWindowHandle"] = winhandle;
#endif

#ifdef Q_WS_X11
        // GLX - According to Ogre Docs:
        // poslong:posint:poslong:poslong (display*:screen:windowHandle:XVisualInfo*)
        QX11Info info =  x11Info ();

        winhandle  = Ogre::StringConverter::toString 
            ((unsigned long)
             (info.display ()));
        winhandle += ":";

        winhandle += Ogre::StringConverter::toString 
            ((unsigned int)
             (info.screen ()));
        winhandle += ":";
        
        winhandle += Ogre::StringConverter::toString 
            ((unsigned long)
             nativewin-> winId());

        //Add the external window handle parameters to the existing params set.
        params["parentWindowHandle"] = winhandle;
#endif

        // Window position to params
        if (left != -1)
            params["left"] = Ogre::StringConverter::toString(left);
        if (top != -1)
            params["top"] = Ogre::StringConverter::toString(top);

        render_window_ = Ogre::Root::getSingletonPtr()-> createRenderWindow(name, width, height, fullscreen, &params);
        return render_window_;
    }

    void ExternalRenderWindow::ResizeWindow(int width, int height)
    {
        if (render_window_)
        {
            render_window_->resize(width, height); 
            render_window_->windowMovedOrResized();
        }
    }

    void ExternalRenderWindow::resizeEvent(QResizeEvent *e)
    {
        ResizeWindow(width(), height());
    }

    void ExternalRenderWindow::keyPressEvent(QKeyEvent *e)
    {
        if(e->modifiers() == Qt::ControlModifier && e->key() == Qt::Key_F)
        {
            if(!isFullScreen())
            {
                showFullScreen();
            }
            else
            {
                showNormal();
            }
        }
    }
}

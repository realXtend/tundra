// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "QOgreUIView.h"
#include "CoreStringUtils.h"
#include "KeyBindings.h"
#include <QDebug>

#ifdef Q_WS_X11
#include <QX11Info>
#endif

#ifdef Q_WS_WIN
#include <windows.h>
#endif

namespace OgreRenderer
{
    QOgreUIView::QOgreUIView (QWidget *parent) : 
        QGraphicsView(parent),
        win_(0),
        view_(0)
    {
        setScene(new QGraphicsScene(this)); // Set parent to scene for qt cleanup
        Initialize_();
    }

    QOgreUIView::~QOgreUIView ()
    {
    }

    void QOgreUIView::Initialize_()
    {
        // Setup QGrapchicsView
        setUpdatesEnabled (false);
        setAutoFillBackground (false);
        setFocusPolicy (Qt::StrongFocus);
        setViewportUpdateMode (QGraphicsView::FullViewportUpdate);
        setHorizontalScrollBarPolicy (Qt::ScrollBarAlwaysOff);
        setVerticalScrollBarPolicy (Qt::ScrollBarAlwaysOff);
        setLineWidth(0);

        // Get rid of extra white layers under widgets
        // Qt doesn't respect the above request to not fill background
        QPalette empty_background = palette ();
        empty_background.setColor (QPalette::Window, Qt::transparent); 
        empty_background.setColor (QPalette::Base, Qt::transparent);
        setPalette (empty_background);

        setAttribute (Qt::WA_OpaquePaintEvent, false);

#ifdef Q_WS_WIN
        // Determine if we are running under windows 7 that has certain paint issues with Qt 4.6 in a QGraphicsView
        // If we are under win 7 we do some dirty hacks to go around this as a temp solution
        OSVERSIONINFOEX os_version_info;
        ZeroMemory(&os_version_info, sizeof(OSVERSIONINFOEX));
        os_version_info.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
        if (GetVersionEx((OSVERSIONINFO *)&os_version_info))
            if (os_version_info.dwMajorVersion == 6 && os_version_info.dwMinorVersion == 1)
                setAttribute(Qt::WA_DontShowOnScreen, true);
#endif

#ifdef Q_WS_MAC
        setAttribute(Qt::WA_PaintOnScreen);
	    setAttribute(Qt::WA_NoSystemBackground);
#endif
    }

    void QOgreUIView::SetWorldView(QOgreWorldView *view) 
    { 
        view_ = view; 
        connect(scene(), SIGNAL( changed(const QList<QRectF> &) ), this, SLOT( SceneChange() )); 
    }

    void QOgreUIView::SetScene(QGraphicsScene *new_scene)
    {
        setScene(new_scene);
        QObject::connect(scene(), SIGNAL( changed (const QList<QRectF> &) ), this, SLOT( SceneChange() ));   
    }

    void QOgreUIView::InitializeWorldView(int width, int height)
    { 
        if (view_) view_->InitializeOverlay(width, height); 
    }

    Ogre::RenderWindow *QOgreUIView::CreateRenderWindow (const std::string &name,  int width, int height, int left, int top, bool fullscreen)
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
            params["left"] = ToString(left);
        if (top != -1)
            params["top"] = ToString(top);

#ifdef USE_NVIDIA_PERFHUD
        params["useNVPerfHUD"] = "true";
        params["Rendering Device"] = "NVIDIA PerfHUD";
#endif

        win_ = Ogre::Root::getSingletonPtr()-> createRenderWindow(name, width, height, fullscreen, &params);
        return win_;
    }

    void QOgreUIView::UpdateKeyBindings(Foundation::KeyBindings *bindings)
    {
        // Python restart QKeySequences
        std::list<Foundation::Binding> bind_list = bindings->GetBindings("python.restart");
        std::list<Foundation::Binding>::const_iterator iter = bind_list.begin();
        std::list<Foundation::Binding>::const_iterator end = bind_list.end();

        if (iter != end)
        {
            python_run_keys_.clear();
            while (iter != end)
            {
                python_run_keys_.append((*iter).sequence);
                iter++;
            }
        }

        // Toggle console QKeySequences
        bind_list = bindings->GetBindings("toggle.console");
        iter = bind_list.begin();
        end = bind_list.end();

        if (iter != end)
        {
            console_toggle_keys_.clear();
            while (iter != end)
            {
                console_toggle_keys_.append((*iter).sequence);
                iter++;
            }
        }
    }

    void QOgreUIView::keyPressEvent (QKeyEvent *e)
    {
/*
        QKeySequence seq(e->key() + e->modifiers());
        if (console_toggle_keys_.contains(seq) && scene()->focusItem())
            emit ConsoleToggleRequest();
        if (python_run_keys_.contains(seq) && scene()->focusItem())
            emit PythonRestartRequest();
*/
        emit ViewKeyPressed(e);
        QGraphicsView::keyPressEvent(e);
    }

    void QOgreUIView::resizeEvent (QResizeEvent *e)
    {
        QGraphicsView::resizeEvent(e);

        // Resize render window and UI texture to match this
        if (view_) 
        {
            view_->ResizeWindow(width(), height());
            view_->ResizeOverlay(viewport()->width(), viewport()->height());
        }
        
        // Resize the scene to view rect
        if (scene())
            scene()->setSceneRect(viewport()->rect());          
    }

    void QOgreUIView::SceneChange()
    {
        setDirty(true);
    }
}

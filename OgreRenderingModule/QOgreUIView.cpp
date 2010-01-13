// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "QOgreUIView.h"

#ifdef Q_WS_X11
#include <QX11Info>
#endif

#ifdef Q_WS_WIN
#include <windows.h>
#endif

namespace OgreRenderer
{
    QOgreUIView::QOgreUIView () :
        QGraphicsView(),
        win_(0),
        view_(0)
    {
        Initialize_();
    }

    QOgreUIView::QOgreUIView (QWidget *parent, QGraphicsScene *scene) : 
        QGraphicsView(scene, parent),
        win_(0),
        view_(0)
    {
        Initialize_();
    }

    QOgreUIView::~QOgreUIView ()
    {
        if (scene())
            delete scene();
    }

    void QOgreUIView::Initialize_()
    {
        bool is_win7 = false;

        #ifdef Q_WS_WIN
        // Determine if we are running under windows 7 that has certain paint issues with Qt 4.6 in a QGraphicsView
        // If we are under win 7 we do some dirty hacks to go around this as a temp solution
        OSVERSIONINFOEX os_version_info;
        ZeroMemory(&os_version_info, sizeof(OSVERSIONINFOEX));
        os_version_info.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
        if (GetVersionEx((OSVERSIONINFO *)&os_version_info))
            if ( os_version_info.dwMajorVersion == 6 && os_version_info.dwMinorVersion == 1)
                is_win7 = true;
        #endif

        QPalette p;
        // Get rid of extra white layers under widgets
        p.setColor(QPalette::Background, Qt::transparent); 
        p.setColor(QPalette::Base, Qt::transparent);
        setPalette(p);

        // Setup QGrapchicsView
        setUpdatesEnabled (false); // Turn off paintEvents, reduces flicker and overpaint
        setFocusPolicy (Qt::StrongFocus);
        setViewportUpdateMode (QGraphicsView::FullViewportUpdate);
        setHorizontalScrollBarPolicy (Qt::ScrollBarAlwaysOff);
        setVerticalScrollBarPolicy (Qt::ScrollBarAlwaysOff);

        if (is_win7)
        {
            // Setup QGrapchicsView some more
            setAutoFillBackground(false);
            setAttribute(Qt::WA_OpaquePaintEvent, false);
            setAttribute(Qt::WA_NoSystemBackground, true);
            setAttribute(Qt::WA_DontShowOnScreen, true); // Hover will stop working but will get rid of the white paint (win7)

            // Setup Viewport
            QWidget *vp = viewport();
            vp->setUpdatesEnabled (false);
            vp->setAutoFillBackground(false);
            vp->setPalette(p);
            vp->setAttribute(Qt::WA_OpaquePaintEvent, false);
            vp->setAttribute(Qt::WA_NoSystemBackground, true);
        }
    }    

    void QOgreUIView::SetWorldView(QOgreWorldView *view) 
    { 
        view_ = view; 
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

        win_ = Ogre::Root::getSingletonPtr()-> createRenderWindow(name, width, height, fullscreen, &params);

        return win_;
    }

    void QOgreUIView::resizeEvent(QResizeEvent *e)
    {
        QGraphicsView::resizeEvent(e);

        // Resize render window and UI texture to match this
        if (view_) 
        {
            view_->ResizeWindow(width(), height());
            view_->ResizeOverlay(viewport()->width(), viewport()->height());
        }
        
        if (scene())
            scene()->setSceneRect(viewport()->rect());
    }

    void QOgreUIView::SceneChange()
    {
        setDirty(true);
    }
}

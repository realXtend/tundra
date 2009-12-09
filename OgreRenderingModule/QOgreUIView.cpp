// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "QOgreUIView.h"

#include <QPaintEvent>

#ifndef Q_WS_WIN
#include <QX11Info>
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

    QOgreUIView::QOgreUIView (QGraphicsScene *scene) : 
        QGraphicsView(scene),
        win_(0),
        view_(0)
    {
        Initialize_();
    }

    QOgreUIView::~QOgreUIView ()
    {
       if ( this->scene() != 0 ) 
            delete this->scene();
    }

    void QOgreUIView::Initialize_()
    {
        setMinimumSize (150,100);

        setUpdatesEnabled (false); // reduces flicker and overpaint

        setFocusPolicy (Qt::StrongFocus);
        setViewportUpdateMode (QGraphicsView::FullViewportUpdate);

        setHorizontalScrollBarPolicy (Qt::ScrollBarAlwaysOff);
        setVerticalScrollBarPolicy (Qt::ScrollBarAlwaysOff);
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
            params["left"] = Core::ToString(left);
        if (top != -1)
            params["top"] = Core::ToString(top);

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

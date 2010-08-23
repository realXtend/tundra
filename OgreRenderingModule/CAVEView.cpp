#include "StableHeaders.h"
#include "CAVEView.h"
#include "Renderer.h"

#include <QResizeEvent>
#include <QKeyEvent>
#include <QDebug>

namespace OgreRenderer
{

    CAVEView::CAVEView(Renderer* renderer)
        :camera_(0),
        render_window_(0)
    {
        renderer_ = renderer;
    }

    CAVEView::~CAVEView()
    {
        renderer_->GetSceneManager()->destroyCamera(camera_);
        Ogre::Root::getSingleton().detachRenderTarget(render_window_->getName());

    }

    void CAVEView::keyPressEvent(QKeyEvent *e)
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

    void CAVEView::Initialize(const QString& name,  Ogre::Vector3 &top_left, Ogre::Vector3 &bottom_left, Ogre::Vector3 &bottom_right, Ogre::Vector3 &eye_pos)
    {
        assert(renderer_);
        Initialize(name, renderer_->GetWindowWidth(), renderer_->GetWindowHeight(),top_left, bottom_left, bottom_right, eye_pos); 

        

    }

    void CAVEView::ReCalculateProjection(Ogre::Vector3 &top_left, Ogre::Vector3 &bottom_left, Ogre::Vector3 &bottom_right, Ogre::Vector3 &eye_pos)
    {

        assert(renderer_);
        assert(camera_);
        assert(render_window_);

        qreal pi = Ogre::Math::PI;

        //Projection magic be happening here.
        double n = camera_->getNearClipDistance();
		double f = camera_->getFarClipDistance();
		double l, r, b, t;
 
        //screenspace axes
        Ogre::Vector3 sr, su, sn;
        //from eye to screenpoints
        Ogre::Vector3 ebl,etl,ebr;

        Ogre::Matrix4 proj_mat, transl, change_base;

        sr=bottom_right-bottom_left;
        sr.normalise();
        su=top_left-bottom_left;
        su.normalise();
        sn=su.crossProduct(sr);
        sn.normalise();

        ebl = bottom_left-eye_pos;
        etl = top_left-eye_pos;
        ebr = bottom_right-eye_pos;

        qreal distance_to_plane = sn.dotProduct(ebl);

        l = sr.dotProduct(ebl)*n/distance_to_plane;
        r = sr.dotProduct(ebr)*n/distance_to_plane;
        b = su.dotProduct(ebl)*n/distance_to_plane;
        t = su.dotProduct(etl)*n/distance_to_plane;

        
        renderer_->GetRoot()->getRenderSystem()->_makeProjectionMatrix(l,r,b,t,n,f,proj_mat);
        change_base=Ogre::Matrix4(sr.x,sr.y,sr.z,0,
                                  su.x,su.y,su.z,0,
                                  sn.x,sn.y,sn.z,0,
                                  0,0,0,1);

        transl.makeTrans(-eye_pos);
        proj_mat = proj_mat*change_base*transl;
        camera_->setCustomProjectionMatrix(true, proj_mat);
        
    }

    
    void CAVEView::Initialize(const QString& name, qreal window_width, qreal window_height, Ogre::Vector3 &top_left, Ogre::Vector3 &bottom_left, Ogre::Vector3 &bottom_right, Ogre::Vector3 &eye_pos)
    {
        assert(renderer_);
        this->setGeometry(20,20,window_width,window_height);
        Ogre::Camera* original_cam = renderer_->GetCurrentCamera();
        std::string std_name = name.toStdString();
        CreateRenderWindow(std_name, window_width, window_height,0,0,false);
        camera_ = new Ogre::Camera(std_name + "_camera", renderer_->GetSceneManager());        
        render_window_->addViewport(camera_);
        camera_->getViewport()->setOverlaysEnabled(false);


        //setup the camera
        camera_->setCustomProjectionMatrix(false);
        camera_->setNearClipDistance(original_cam->getNearClipDistance());
        camera_->setFarClipDistance(original_cam->getFarClipDistance());
        
        Ogre::SceneNode* node = dynamic_cast<Ogre::SceneNode*>(original_cam->getParentNode());
        if(node)
        {
            node->attachObject(camera_);
        }

       
        ReCalculateProjection(top_left, bottom_left, bottom_right, eye_pos);

    }

    void CAVEView::ResizeWindow(int width, int height)
    {
        if (render_window_)
        {
            render_window_->resize(width, height); 
            render_window_->windowMovedOrResized();
        }
    }

    void CAVEView::resizeEvent(QResizeEvent *e)
    {
        ResizeWindow(width(), height());
    }



    Ogre::RenderWindow* CAVEView::CreateRenderWindow(const std::string &name,  int width, int height, int left, int top, bool fullscreen)
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

        render_window_ = Ogre::Root::getSingletonPtr()-> createRenderWindow(name, width, height, fullscreen, &params);

        return render_window_;

    }
}
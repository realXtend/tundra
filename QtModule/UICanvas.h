// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_QtModule_UICanvas_h
#define incl_QtModule_UICanvas_h

#include <Ogre.h>

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QWidget>
#include <QRect>
#include <QDrag>

namespace QtUI
{
    /**
     *  UICanvas is interface which must be used to show own UI components. How UI-component is show depends solely which mode UICanvas is set. If UICanvas object
     *  is set as External mode then UI component will be shown as external window. Case when mode is set as Internal canvas is shown as top of Ogre render window as overlay, when 
     *  mode is an Embedded UI component is shown in 3D-world as texture of some Ogre::Entity, binding UICanvas to entity must be done by class user.  
     *  
     */

    class UICanvas : public QGraphicsView
    {
         Q_OBJECT
         Q_PROPERTY(Mode mode READ GetMode WRITE SetMode)
         Q_ENUMS(Mode)
 
    public:
        
        enum Mode { External = 0, Internal, Embedded };
        
        UICanvas();
        UICanvas(Mode mode);

        virtual ~UICanvas();
        
        void SetMode(Mode mode) { mode_ = mode; }
        Mode GetMode() const { return mode_; }

        //QGraphicsScene* GetScene() { return scene_;}
      
        /// Call this to render the canvas onto the Ogre surface that is associated to this view.
        /// OgreUIView manages the Ogre overlays by itself so after calling this the new updated
        /// canvas is automatically shown.
        void RenderSceneToOgreSurface();

        /// Sends a MouseMove message to the associated QGraphicsScene widget.
        /// This function is for both hover and drag messages.
        void InjectMouseMove(int x, int y);

        /// Sends a MousePress message to the associated QGraphicsScene widget.
        /// A corresponding MouseRelease needs to be sent afterwards.
        void InjectMousePress(int x, int y);

        /// Sends a MouseRelease message on the QGraphicsScene in the given position.
        void InjectMouseRelease(int x, int y);

        /// Resizes the UI canvas size and reallocates Qt and Ogre surfaces to the new size.
        void SetViewCanvasSize(int width, int height);

        /// Call this to pass in the size of the owning window.
        void SetParentWindowSize(int windowWidth, int windowHeight);

        /// @return The Ogre overlay container associated to this view.
        Ogre::OverlayContainer *GetContainer() const { return container_; }
    	
	    void SetDirtyState(bool state) { view_dirty_ = state; }

    public slots:
    	
	    void Update();

    protected:
        /// Override Qt's QGraphicsView's background drawing to enable alpha on the empty
        /// areas of the canvas.
        void drawBackground(QPainter *painter, const QRectF &rect);

    private:
        /// Creates an Ogre Overlay and an Ogre Texture surface of the given size.
        void CreateOgreResources(int width, int height);

        /// Recreates an Ogre texture when the canvas size needs to change.
        void ResizeOgreTexture(int width, int height);

        ///\todo There might be a way to utilize the way that Qt's QGraphicsView
        /// stores these parameters, but they're currently internal/private so
        /// need to track this ourselves.
        ///\todo Possibly refactor these from here to QtModule.cpp to store?
        /// These issues will be revisited when implementing multiple scenes/views.
        int mousePressX;
        int mousePressY;
        bool mouseDown;
	    bool view_dirty_;

        Ogre::TexturePtr texture_;
        Ogre::MaterialPtr material_;
        Ogre::Overlay *overlay_;
        Ogre::OverlayContainer *container_;

        //QGraphicsScene* scene_;
        Mode mode_;

    };


}

#endif

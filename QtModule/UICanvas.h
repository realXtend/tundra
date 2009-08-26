// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_QtModule_UICanvas_h
#define incl_QtModule_UICanvas_h

#include <Ogre.h>


#include <QGraphicsView>
#include <QSize>
#include <QRect>
#include <QString>
#include "QtModuleApi.h"

class QPainter;
class QWidget;
class QGraphicsProxyWidget;

namespace QtUI
{
    /**
     *  UICanvas is interface which must be used to show own UI components. How UI-component is show depends solely which mode UICanvas is set. If UICanvas object
     *  is set as External mode then UI component will be shown as external window. Case when mode is set as Internal canvas is shown as top of Ogre render window as overlay, when 
     *  mode is an Embedded UI component is shown in 3D-world as texture of some Ogre::Entity, binding UICanvas to entity must be done by class user. Currently changes of UICanvas 
     *  modes (external to internal or otherway) is not supported. 
     *  
     */

    class QT_MODULE_API UICanvas : public QGraphicsView
    {
         Q_OBJECT
         Q_PROPERTY(Mode mode READ GetMode WRITE SetMode)
         Q_ENUMS(Mode)
 
    public:
        
        enum Mode { External = 0, Internal, Embedded };
        
        UICanvas();
        UICanvas(Mode mode, const QSize& parentWindowSize);

        virtual ~UICanvas();
        
        void SetMode(Mode mode) { mode_ = mode; }
        Mode GetMode() const { return mode_; }

        /**
         * Add windget into canvas. Canvas will take ownership of the given widget. 
         * @param widget anykind QWidget. 
         */
        void AddWidget(QWidget* widget);  

        /** 
         * Sets a position of canvas in render-window coordinate system.
         * 
         * @note if canvas mode is @p External canvas position is not tight to render window 
         * 
         * @param x is x-axis coordinate where axis origo lies top-left of render window in external case uses Qt window frame-system.  
         * @param y is y-axis coordinate where axis origo lies top-left of render window in external case uses Qt window frame-system.
         * 
         */
        void SetPosition(int x, int y);
        
        /**
         * Returns a position of canvas in render-window coordinate frame. This is valid only for canvases
         * which works in Internal or Embedded -mode. 
         * @note origo lies top-left of render window. 
         **/
        
        QPointF GetPosition() const;

        
        /**
         * Resizes the UI canvas size and reallocates Qt and Ogre surfaces to new size. 
         * @param width is a new width of canvas. 
         * @param height is a new height of canvas.
         * @todo override widgets own resize?
         */
        void SetCanvasSize(int width, int height);

        
        /**
         * Returns a overlay container which contains overalay which are associated to this canvas.
         * @return The Ogre overlay container associated to this canvas. if canvas is in external mode it returns zero pointer.
         *
         */
        
        Ogre::OverlayContainer *GetContainer() const { return container_; }
    	
        /**
         * Returns canvas uniq id which is used to generate canvases uniq texture and material name. Texture name is "tex" + id and material name is "mat" + id. Container name is "con" + id
         * and overlay name is "over" + id
         *
         * @return canvas uniq ID.
         */
        QString GetID() const { return id_;}

        /**
         * Shows widget
         */
        void Show();

        /**
         * Hides widget
         *
         */
        void Hide();
    
        /**
         * Maps given point (assumption that it is really on canvas area) from render window coordinates to view coordinates (viewport). 
         * @return coordinates in view (viewport) coordinate system.
         */
        QPoint MapToCanvas(int x, int y);

        /**
         * Returns current renderwindow size. 
         * @return current renderwindow size. 
         */
        QSize GetRenderWindowSize() const { return renderWindowSize_;}

     public slots:
    	
	    /** 
         * When Render() is called canvas is rendered, this happen if and only if canvas is on dirty state (meaning that there is something new to draw).
         */
        void Render();

        /**
         * Sets canvas to dirty-state, if it is true then it should be draw.  
         */
        void Dirty() { dirty_ = true; }

        /**
         * Sets new Render-window size. 
         */
        void SetRenderWindowSize(const QSize& size) { renderWindowSize_ = size; }

        

    signals:
        
        /** 
         * Signal is emited if canvas Z-order in Ogre scene might change. 
         */
        void RequestArrange();

    protected:
        
        /// Override Qt's QGraphicsView's background drawing to enable alpha on the empty
        /// areas of the canvas.

        void drawBackground(QPainter *painter, const QRectF &rect);

        /// Call this to render the canvas onto the Ogre surface that is associated to this view.
        /// OgreUIView manages the Ogre overlays by itself so after calling this the new updated
        /// canvas is automatically shown.
        
        void RenderSceneToOgreSurface();


    private:
        
        /// Creates an Ogre Overlay and an Ogre Texture surface of the given size.
        void CreateOgreResources(int width, int height);

        /// Recreates an Ogre texture when the canvas size needs to change.
        void ResizeOgreTexture(int width, int height);

        /// Defines that is scene dirty -- should it draw again. 
	    bool dirty_;

        Ogre::TexturePtr texture_;
        Ogre::MaterialPtr material_;
        
        Ogre::Overlay *overlay_;
        Ogre::OverlayContainer *container_;

        QSize renderWindowSize_;
        Mode mode_;
        QString id_;
        int widgets_;

        QList<QGraphicsProxyWidget* > scene_widgets_;

    };


}

#endif

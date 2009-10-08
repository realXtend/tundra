// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_QtModule_UICanvas_h
#define incl_QtModule_UICanvas_h


#include <QGraphicsView>
#include <QSize>
#include <QRect>
#include <QString>
#include <QTimer>
#include "QtModuleApi.h"

class QPainter;
class QWidget;
class QGraphicsProxyWidget;

namespace Ogre
{
    class OverlayContainer;
    class Overlay;
}

namespace QtUI
{
    /**
     *  UICanvas is interface which must be used to show own UI components. How UI-component is show depends solely which mode UICanvas is set. If UICanvas object
     *  is set as External mode then UI component will be shown as external window. Case when mode is set as Internal canvas is shown as top of Ogre render window as overlay, when 
     *  or in 3D-world as texture of some Ogre::Entity (binding UICanvas to entity must be done by class user). Currently changes of UICanvas 
     *  modes (external to internal or otherway) is not supported. 
     *  
     */

    class QT_MODULE_API UICanvas : public QGraphicsView
    {
         Q_OBJECT
         Q_PROPERTY(Mode mode READ GetMode WRITE SetMode)
         Q_ENUMS(Mode)
 
    public:
        
        enum Mode { External = 0, Internal};
        enum CanvasSide { Left = 0, Right, Bottom, Top };

        UICanvas();
        UICanvas(Mode mode, const QSize& parentWindowSize);

        virtual ~UICanvas();
        
        void SetMode(Mode mode) { mode_ = mode; }
        Mode GetMode() const { return mode_; }

    
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
         * Locks canvas position so that it cannot drag over render windows.
         * @note this behavior is only possible for internal canvases. 
         * @param locked is a boolean which defines that is canvas locked. 
         */
         
        void SetLockPosition(bool locked) { locked_ = locked; }

        /**
         * Returns true if canvas is locked
         * false if not. 
         */

        bool IsCanvasPositionLocked() const { return locked_; }

        /**
         * Resizes the UI canvas size and reallocates Qt and Ogre surfaces to new size. 
         * @param width is a new width of canvas. 
         * @param height is a new height of canvas.
         * @todo override widgets own resize?
         */
        void SetCanvasSize(int width, int height);
        QSize GetSize() const { return this->size(); }

        /** 
         * Resizes canvas. 
         * @param height new height
         * @param width new width
         * @param side is which side of canvas will be resized.
         */
        
        void Resize(int height, int width, CanvasSide side);

        /** 
         * Returns canvas geometry. 
         */
        QRect GetCanvasGeometry() const { return this->geometry(); }
    	
        /**
         * Returns canvas uniq id which is used to generate canvases uniq texture and material name. Texture name is "tex" + id and material name is "mat" + id. Container name is "con" + id
         * and overlay name is "over" + id. 
         * @note for External widget there exist no material or overlay, container. 
         * @return canvas uniq ID.
         */
        QString GetID() const { return id_;}

       
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


        /**
         * Sets this widget as a top widget on Ogre renderwindow. Note this works only for internal windows.
         */
        void SetTop();

        /**
         * Sets this widget as a back widget on Ogre renderwindow. Note this works only for internal windows.
         */
        void SetBack();


        /**
         * Set Z-order. Changes overlay own "default" z-order value to new value.
         * @note by default overlay z-value is 1
         * @param order new order value must be under 650 (this comes from Ogre).
         *
         */
        void SetZOrder(int order);
        
        /**
         * Returns overlay z-order value. 
         * @return overlay z-order value and for external canvases it returns -1
         */
        int GetZOrder() const;

        /** 
         * Returns true if canvas is not shown in window. 
         */
        bool IsHidden() const;

        /**
         * Activates canvas. This is a HACK-way to cheat Qt to think that 
         * canvas is shown even that it is not actually shown. 
         * @note this works currently only in windows
         */
        void Activate();

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
        void SetRenderWindowSize(const QSize& size); 

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
         * Add windget into canvas. Canvas will take ownership of the given widget. 
         * @param widget anykind QWidget. 
         */
        void AddWidget(QWidget* widget); 

		/**
		 * Set the window title with QString &title
		 */
		void SetCanvasWindowTitle(QString title);

		/**
		 * Set the window icon with Qicon &icon
		 */
		void SetCanvasWindowIcon(QIcon &icon);


    signals:
                
        /**
         * Signal is emited if this canvas need to be top. 
         */
        void ToTop(const QString& id);

        /**
         * Signal is emited if this canvas need to be behind. 
         */
        void ToBack(const QString& id);


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

        /// Ogre texture name. 
        QString surfaceName_;
        
        Ogre::Overlay *overlay_;
        Ogre::OverlayContainer *container_;

        QSize renderWindowSize_;
        Mode mode_;
        QString id_;
        int widgets_;
        bool locked_;
      
        QList<QGraphicsProxyWidget* > scene_widgets_;

       
       
    };


}

#endif

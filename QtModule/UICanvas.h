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
    /** UICanvas represents a rectangular-shaped 2D canvas on which Qt widges can be placed. To support 
        proper headless execution mode and canvas compositing effects, clients should always use these 
        canvases as containers for their own widgets and never instantiate QWidgets as standalone windows.

        To add a widget to a canvas, allocate it dynamically and call UICanvas::AddWidget. UICanvas takes
        ownership of the widget memory and stores it to a QGraphicsScene.
        
        There are two main display modes a UICanvas can be initialized in: UICanvas::Internal and 
        UICanvas::External. In Internal mode, the canvas creates and maintains its own render surface to 
        which the canvas widgets are drawn on. This surface can then be shown in 2D with desktop 
        compositing effects (e.g. alpha fade), or used as a texture for a 3D object in the scene.

        In External mode, the canvas doesn't create a 3D texture surface, but just creates an external
        window that contains a QGraphicsView widget that displays the internal QGraphicsScene containing
        all the widgets that have been added to the UICanvas. This functionality is mostly intended for
        debugging, but can perhaps be useful in other cases as well.

        Changing the UICanvas display mode after the UICanvas has been instantiated is not supported. */
    class QT_MODULE_API UICanvas : public QGraphicsView
    {
        Q_OBJECT
        Q_PROPERTY(Mode mode READ GetMode WRITE SetMode)
        Q_ENUMS(Mode)


    public:
        enum Mode { External = 0, Internal};
   
        enum CanvasCorner { TopLeft = 0, BottomLeft, TopRight, BottomRight };

        UICanvas();
        UICanvas(Mode mode, const QSize& parentWindowSize);

        virtual ~UICanvas();

        Mode GetMode() const { return mode_; }
    
        /** Sets the canvas position relative to the parent window. If the canvas is in internal mode,
            the parent is the main render window, otherwise the it is the desktop. (0,0) denotes the upper
            left corner. */
        void SetPosition(int x, int y);
        
        /** @return The position of the canvas relative to the parent window. @see SetPosition. */
        QPointF GetPosition() const;

        /** Locks the canvas position so that it cannot be moved.
            @note This function is only applicable if the canvas is in internal display mode. */
        void SetLockPosition(bool locked) { locked_ = locked; }

        /** @return True if the canvas position has been locked.
            @note This function is only applicable if the canvas is in internal display mode. */
        bool IsCanvasPositionLocked() const { return locked_; }

        /** Resizes the UI canvas size and reallocates Qt and Ogre surfaces to new size. 
            @param width is a new width of canvas. 
            @param height is a new height of canvas.
            @todo override widgets own resize? */
        void SetCanvasSize(int width, int height);

        /// @return A pair (width, height) containing the absolute size of the canvas.
        QSize GetSize() const { return this->size(); }



        /**
            Resizes the canvas. This function does not just alter the width and the height
            of the canvas, but also repositions the canvas according to the anchor parameter.
            This is to support resizing the way a user expects when dragging a corner or
            a side of a canvas.
            @param width The new width in absolute pixels.
            @param height The new height in absolute pixels.
            @param anchor This parameter denotes which corner of the canvas is kept unmoved.
                That is, the shrinking or expanding of the canvas will be performed on
                the two opposing edges of the canvas.
        */
        void Resize(int width, int height, CanvasCorner anchor);
        


        /// @return (x,y,w,h) containing the absolute position and size of the canvas, relative
        /// the upper-left of the main render window.
        QRect GetCanvasGeometry() const { return this->geometry(); }
    	
        /** The Ogre resources associated to this canvas have the following names: 
            Texture: "tex" + id.
            Material: "mat" + id. 
            Overlay container: "con" + id.
            @return The unique id associated to this canvas. */
        QString GetID() const { return id_;}
       
        /** Maps the given point (assumes that it really is inside this canvas area) from render 
            window coordinates to canvas local coordinates.
            @return (x,y) coordinates of the given point relative to the canvas upper-left. */
        QPoint MapToCanvas(int x, int y);

        /// @return The size of the main render window, in absolute pixel units.
        QSize GetRenderWindowSize() const { return renderWindowSize_;}

        /// Brings this canvas to the front in the Z order of the internal canvases. Note that 
        /// this works only for internal canvases.
        void SetTop();

        /// Pushes this canvas to last in the Z order of the internal canvases. Note that this 
        /// works only for internal canvases.
        void SetBack();

        /** Sets the Z order of this canvas. Changes overlay own "default" z-order value to new 
            value. \todo This function is pending to be removed in favor of 'BringToFront()', 
            'PushToBack()' and 'SetAlwaysOnTop()' functions, which are more intuitive and simpler 
            to manage than absolute z values.
            @note by default overlay z-value is 1
            @param order The new Z order value, in the range [1, 650[. (this comes from Ogre). 
                The larger the number is, the higher on the z order list this canvas is. */
        void SetZOrder(int order);
        
        /// @return The z order value of this canvas, or -1 if this canvas is in External mode.
        int GetZOrder() const;

        /// @return True if this canvas is hidden, false otherwise.
        bool IsHidden() const;

        /**
         * Activates canvas. This is a HACK-way to cheat Qt to think that 
         * canvas is shown even that it is not actually shown. 
         * @note this works currently only in windows
         */
        void Activate();

     public slots:
    	
        /// Redraws this internal canvas to an Ogre surface if it has become dirty. 
        /// For external canvases, does nothing.
        void Render();

        /// Marks the Ogre surface of this canvas to be pending for redraw.
        /// For external canvases, does nothing.
        void SetDirty() { dirty_ = true; }

        /// Recalculates the relative normalized canvas overlay position and size values needed 
        /// for Ogre display. Called whenever the main render window size changes.
        void SetRenderWindowSize(const QSize& size); 

        /// Shows this canvas.
        void Show();

        /// Hides the whole canvas.
        void Hide();

        /// Adds a widget into this canvas.
        /** Call this method to add any type of widget into the QGraphicsScene contained in this 
            canvas. This canvas will take the ownership of the widget and free it when appropriate. */
        void AddWidget(QWidget* widget); 

        /// Sets the window title. For internal canvases the title is not shown, so this has no effect.
		void SetCanvasWindowTitle(QString title);

        /// Sets the window icon. For internal canvases the title is not shown, so this has no effect.
		void SetCanvasWindowIcon(QIcon &icon);

    signals:                
        /// Emitted when this canvas needs to be brought to top in the Z order. UIController tracks 
        /// this signal and performs the necessary reordering.
        void ToTop(const QString& id);

        /// Emitted when this canvas needs to be brought to bottom in the Z order. UIController 
        /// tracks this signal and performs the necessary reordering.
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
        /// Marked as private since we don't support changing the display mode after the canvas has 
        /// been instantiated. \todo this function is quite obsolete, remove or implement support for 
        /// changing the mode.
        void SetMode(Mode mode) { mode_ = mode; }

        /// Creates an Ogre Overlay and an Ogre Texture surface of the given size.
        void CreateOgreResources(int width, int height);

        /// Recreates an Ogre texture when the canvas size needs to change.
        void ResizeOgreTexture(int width, int height);

        /// Tracks whether the Ogre surface associated to this canvas is dirty and should be redrawn.
	    bool dirty_;

        /// The name of the Ogre texture associated with this canvas. Not used if mode_=External.
        QString surfaceName_;

        /// The Overlay object used to composit this canvas onto the 3D screen. Not used if mode_=External.
        Ogre::Overlay *overlay_;
        /// The OverlayContainer object used to composit this canvas onto the 3D screen. Not used if mode_=External.
        Ogre::OverlayContainer *container_;

        /// The size of the main render window. Required to be able to compute the proper normalized
        /// sizes and positions for the Ogre overlays. Not used if mode_=External.
        QSize renderWindowSize_;

        /// Denotes whether this canvas is composited using Ogre overlays (Internal mode), or the OS
        /// windowing system (External mode).
        Mode mode_;
        
        /// The unique canvas ID of this canvas.
        QString id_;

        /// The number of widgets that have been added to this canvas.
        int widgets_;
        
        /// If true, this canvas cannot be moved by dragging the mouse on the window title. Only functional if
        /// mode=Internal.
        bool locked_;
      
        /// Contains the widget proxies of all the widgets that have been added to this canvas.
        QList<QGraphicsProxyWidget*> scene_widgets_;
    };
}

#endif

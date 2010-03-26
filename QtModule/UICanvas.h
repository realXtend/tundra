// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_QtModule_UICanvas_h
#define incl_QtModule_UICanvas_h



#include <QSize>
#include <QRect>
#include <QString>
#include <QTimer>
#include <QTime>
#include "UILocationPolicy.h"
#include "UIAppearPolicy.h"
#include "UIGraphicsView.h"
#include "QtModuleApi.h"

class QPainter;
class QWidget;
class QGraphicsProxyWidget;



namespace Ogre
{
    class OverlayContainer;
    class Overlay;
    class TextureUnitState;
}

namespace QtUI
{
  
/** UICanvas represents a rectangular-shaped 2D canvas on which Qt widgets can be placed. To support 
    proper headless execution mode and canvas compositing effects, clients should always use these 
    canvases as containers for their own widgets and never instantiate QWidgets as standalone windows.

    To add a widget to a canvas, allocate it dynamically and call UICanvas::AddWidget. UICanvas takes
    ownership of the widget memory and stores it to a QGraphicsScene. Do not create stack-allocated
    widgets.

    There are two main display modes a UICanvas can be initialized in: UICanvas::Internal and 
    UICanvas::External. In Internal mode, the canvas creates and maintains its own render surface to 
    which the canvas widgets are drawn on. This surface can then be shown in 2D with desktop 
    compositing effects (e.g. alpha fade), or used as a texture for a 3D object in the scene.

    In External mode, the canvas doesn't create a 3D texture surface, but just creates an external
    window that contains a QGraphicsView widget that displays the internal QGraphicsScene containing
    all the widgets that have been added to the UICanvas. This functionality is mostly intended for
    debugging, but can perhaps be useful in other cases as well.

    Currently changing the UICanvas display mode after the UICanvas has been instantiated is not supported. */
    class QT_MODULE_API UICanvas :  public QObject
    {
        Q_OBJECT

    public:
        /** The display mode defines whether this canvas is shown in Internal mode, i.e. embedded into the main
            2D window, or in External mode, where the canvas will have its own window that is separate from the
            main render window. */
        enum DisplayMode { External = 0, Internal};

    private:
        /** Ctor for a creating a new UICanvas. Do not instantiate UICanvases directly, but use the 
            UIController::CreateCanvas factory method instead.
            @param mode The display mode to use for this canvas.
            @param parentWindowSize The size of the main render window that in which this canvas is embedded to.
                In External display mode, this parameter is ignored. */
        UICanvas(DisplayMode mode, const QSize& parentWindowSize);

    public:
        Q_PROPERTY(DisplayMode mode READ GetDisplayMode WRITE SetDisplayMode)
        Q_ENUMS(DisplayMode)

        ~UICanvas();
        
        /** Sets the display mode for this canvas.
            @see GetDisplayMode for getting current display mode. 
            @param mode The new display mode to use. */
        void SetDisplayMode(DisplayMode mode) { mode_ = mode; }

        /** @return The display mode that the this canvas is in.
            @see SetDiplayMode */
        DisplayMode GetDisplayMode() const { return mode_; }

        /** @return True if the position of this canvas is locked and it cannot be moved by 
            dragging the title bar of the canvas. Note that this does not block SetPosition
            from being used.
            @see SetLocationPolicy for changing the canvas location behaviour. 
            @note This function is only applicable if the canvas is in internal display mode. */
        bool IsPositionStationary() const { return locationPolicy_->IsStationary(); } 
       
        /** @return True if the user can resize the canvas by dragging from the canvas edges.
            @note This function is only applicable if the canvas is in internal display mode. */
        bool IsResizable() const { return appearPolicy_->IsResizable(); }
        
        /** @return True if this canvas is always kept above all other canvases in 
            the compositing Z order. Use SetAlwaysOnTop to change this. */
        bool IsAlwaysOnTop() const { return locationPolicy_->IsAlwaysOnTop(); }

        /** @return The position of this canvas relative to the parent window. 
         * @see SetPosition for changing canvas position. */
        QPointF GetPosition() const;

        /// Identifies a corner of a widget. Used to specify an anchor point for resizing the canvas.
        enum Corner { TopLeft = 0, BottomLeft, TopRight, BottomRight };
        
        /** Resizes the canvas. This function does not just alter the width and the height
            of the canvas, but also repositions the canvas according to the anchor parameter.
            This is to support resizing the way the user expects when dragging a corner or
            a side of a canvas.
            @param width The new width in absolute pixels.
            @param height The new height in absolute pixels.
            @param anchor This parameter denotes which corner of the canvas is kept unmoved.
                That is, the shrinking or expanding of the canvas will be performed on
                the two opposing edges of the canvas. */
        void Resize(int width, int height, Corner anchor = TopLeft);

        /// @return The unique pseudorandomly generated id associated to this canvas. 
        /** The Ogre resources associated to this canvas have the following names: 
            Texture: "tex" + id.
            Material: "mat" + id. 
            Overlay container: "con" + id. */
        QString GetID() const { return id_;}
     
        /** @return The UIGraphicsView associated to this canvas. This view is used
            to render the contents of the QGraphicsScene. */
        UIGraphicsView* GetView() { return view_; }

        /** Maps the given point from render window coordinates to canvas local coordinates.
            This function assumes that these coordinates do lie inside this canvas.
            @return The (x,y) coordinates of the given point relative to the canvas upper-left. */
        QPoint MapToCanvas(int x, int y);

        /// @return True if this canvas is the currently active canvas.
        bool IsActiveWindow() const { return view_->isActiveWindow(); }

        /// @return True if this canvas is the one that currently has the keyboard focus.
        bool HasFocus() const { return view_->hasFocus(); }

        /// @return The size of the main render window, in absolute pixel units.
        QSize GetRenderWindowSize() const { return renderWindowSize_;}
      
        void SetLocationPolicy(UILocationPolicy* policy);
        UILocationPolicy* GetLocationPolicy() { return locationPolicy_; }
       
        void SetAppearPolicy(UIAppearPolicy* policy); 
        UIAppearPolicy* GetAppearPolicy() { return appearPolicy_; }
        
        /// @return A pair (width, height) of the absolute pixel size of the canvas.
        QSize GetSize() const { return view_->size(); }

        /** @return A tuple (x,y,w,h) containing the absolute position and size of the 
            canvas, relative the upper-left of the main render window. These are in 
            pixels. */
        QRect GetCanvasGeometry() const { return view_->geometry(); }

    public slots:

        /** Brings this canvas to the front in the Z order of the internal canvases. Note that 
            this works only for internal canvases. */
        void BringToTop();

        /** Pushes this canvas to last in the Z order of the internal canvases. Note that this 
            works only for internal canvases. */        
        void PushToBack(); 

         /** Resizes this canvas and reallocates Qt and Ogre surfaces to the new size.
            @param width The new width of canvas, in pixel units.
            @param height The new height of canvas, in pixel units. */
        void SetSize(int width, int height);

        /** Sets the canvas position relative to the parent window. If the canvas is in 
            internal mode, the parent is the main render window, otherwise the it is the 
            desktop. (0,0) denotes the upper left corner. The coordinates are in pixel units. */        
        void SetPosition(int x, int y);

        /** Sets whether this canvas is stationary, i.e. it cannot be moved by the user by dragging
            the title bar. This only applies for canvases in insternal mode. */
        void SetStationary(bool enable) { locationPolicy_->SetStationary(enable); }

        /** Sets whether the user can resize this canvas by dragging from the edges. This
            does not block SetCanvasSize function from being used. */
        void SetResizable(bool enable) { appearPolicy_->SetResizable(enable); }

        /** Sets whether this canvas is shown always above all other canvases. For external canvases
            this has no effect. */
        void SetAlwaysOnTop(bool on) { locationPolicy_->SetAlwaysOnTop(on); }

        /// @return True if this canvas is hidden, false otherwise.
        bool IsHidden() const;
        
        /** Marks the Ogre surface of this canvas to be pending for redraw.
            For external canvases, does nothing. */
        void Redraw() { dirty_ = true; }

        /// Shows this canvas.        
        void Show();

        /// Hides this canvas.        
        void Hide();

        /** Adds a widget into this canvas.
            Call this method to add any type of widget into the QGraphicsScene contained in this
            canvas. This canvas will take the ownership of the widget and free it when appropriate. */
        void AddWidget(QWidget* widget);

        /** Adds a proxy widget into the canvases scene 
            This method is used when transforming QGraphicsProxyWidgets from normal UI scene to 3D canvas scene */
        void AddProxyWidget(QGraphicsProxyWidget *proxy_widget);

        /** Gets, removes and returns Canvases proxy widget
            This is used only when we want to return the QGraphicsProxyWidget to the 2D scene from 3D canvas scene */
        QGraphicsProxyWidget *Remove3DProxyWidget();

        /// Sets the window title. For internal canvases the title is not shown, so this has no effect.
        void SetWindowTitle(const QString& title);

        /// Sets the window icon. For internal canvases the title is not shown, so this has no effect.
        void SetWindowIcon(const QIcon &icon);

    signals:
        /// Emitted when this canvas moves to top in the Z order.      
        void ToTop(const QString& id);

        /// Emitted when this canvas moves to bottom in the Z order. 
        void ToBack(const QString& id);

        /** Emitted when renderer window size is changed so individual widgets may know this and reposition accordingly
            @param size is a new size of render window. */
        void RenderWindowSizeChanged(const QSize& size);

        /** Emitted when this canvas is hidden. */
        void Hidden();

        /** Emitted when this canvas is shown. */
        void Shown();
        
    private slots:
        /** Recalculates the relative normalized canvas overlay position and size values needed 
            for Ogre display. Called whenever the main render window size changes. */
        void SetRenderWindowSize(const QSize& size); 

        /**
         * Used to resize widgets inside of scene in case of external display mode. 
         * @param event is resize event. 
         */
        void ResizeEvent(QResizeEvent* event);

    private:
        UICanvas(const UICanvas &); ///< UICanvas is noncopyable.
        void operator=(UICanvas &); ///< UICanvas is noncopyable.

        /// Override Qt's QGraphicsView's background drawing to enable alpha on the empty
        /// areas of the canvas.
        void drawBackground(QPainter *painter, const QRectF &rect);

        /** Renders the canvas onto a Ogre GPU surface that is associated with this view. For
            external canvases, this does nothing. */
        void RenderSceneToOgreSurface();

        friend class UIController;

        /// Makes this canvas to be the currently active canvas in the OS windowing system.
        void Activate();

        /// Creates an Ogre Overlay and an Ogre Texture surface of the given size.
        void CreateOgreResources(int width, int height);

        /// Recreates an Ogre texture when the canvas size needs to change.        
        void ResizeOgreTexture(int width, int height);

        /// Resizes scene widgets when canvas size has been changed        
        void ResizeWidgets(int width, int height);
 
		/** Redraws this internal canvas to an Ogre surface if it has become dirty. 
            For external canvases, does nothing. */
        void Render();
 
        /** Sets the Ogre overlay Z-order of this canvas. 
            @param order The new Z order value, in the range [1, 650[. (this comes from Ogre). */
        void SetZOrder(int order);        
         
        /// @return The z order value of this canvas, or -1 if this canvas is in External mode.
        int GetZOrder() const;

        /// Tracks whether the Ogre surface associated to this canvas is dirty and should be redrawn.
	    bool dirty_;
	    
        /// Renderwindow size changed-flag. On next frame, textures may still be garbled, so we rerender canvas twice.
        /// \todo Hack, and will be removed when a proper fix is found.
	    bool renderwindow_changed_;

        /// The name of the Ogre texture associated with this canvas. Not used if mode_=External.
        QString surfaceName_;

        /// The Overlay object used to composit this canvas onto the 3D screen. Not used if mode_=External.
        Ogre::Overlay *overlay_;

        /// The OverlayContainer object used to composit this canvas onto the 3D screen. Not used if mode_=External.
        Ogre::OverlayContainer *container_;

        /// Ogre texture unit state. 
        Ogre::TextureUnitState* state_;

        /// The size of the main render window. Required to be able to compute the proper normalized
        /// sizes and positions for the Ogre overlays. Not used if mode_=External.
        QSize renderWindowSize_;

        /// Denotes whether this canvas is composited using Ogre overlays (Internal mode), or the OS
        /// windowing system (External mode).
        DisplayMode mode_;
        
        /// The unique canvas ID of this canvas.
        QString id_;
        
        UILocationPolicy* locationPolicy_;
        UIAppearPolicy* appearPolicy_;

        UIGraphicsView* view_;

        /// Contains the widget proxies of all the widgets that have been added to this canvas.
        QList<QGraphicsProxyWidget*> scene_widgets_;

        /// A temporary QImage surface on which the QGraphicsScene is drawn on before pushing it through to the GPU.
        QImage scratchSurface;
    };
}

#endif

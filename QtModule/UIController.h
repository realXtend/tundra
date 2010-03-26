// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_QtModule_UIController_h
#define incl_QtModule_UIController_h

#include "UICanvas.h"
#include "Core.h"

#include <QList>
#include <QPoint>
#include <QTime>
#include <QGraphicsSceneMouseEvent> 
#include <QKeyEvent>

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

namespace QtUI
{
    class QtModule;

    /** UIController owns all the UICanvases and manages the injection of mouse and keyboard 
        inputs into those. It also handles the compositing logic (Z-ordering, show/hide)
        needed to manage the distinct canvases on the 2D screen. This class is internal
        to the QtModule and should not be instantiated elsewhere. */
    class UIController : public QObject
    {   
        Q_OBJECT
    
    public:
        UIController();
        ~UIController();

        /** Creates a new UICanvas. The UICanvas owns a QGraphicsScene in which Qt widgets 
            can be added to.
            @param mode The canvas compositing mode to be used with the new canvas. */
        boost::weak_ptr<UICanvas> CreateCanvas(UICanvas::DisplayMode mode = UICanvas::Internal);

        /// @return The size of the main render window in which all the 2D composited windows lie.
        QSize GetParentWindowSize() const { return parentWindowSize_;}

        /** Removes the canvas with the given ID from the internal list of canvases. This will cause a
            destruction of the UICanvas if the refcount drops to zero. Make sure that you do not
            keep a shared_ptr to a canvas that has already been removed, but only store weak_ptrs to
            be able to check when a canvas has died. */
        void RemoveCanvas(const QString& id);

        /// @return True if UIController expects that the OIS keyboard system should now be in buffered mode.
        bool IsKeyboardFocus() const { return keyboardFocusCanvas != 0; }

        /** Checks if the given canvas contains the given point.
            @param point An (x,y) pair in main render window coordinates.
            @return True if the given point is inside the surface area of the given canvas. */
        bool Contains(const UICanvas &canvas, const QPoint &point) const;
		
		/** @return A pointer to a canvas with the given id, or a null pointer if no canvas with that id exists.
            Do not upcast this to a shared_ptr and then store it in a client module memory area. Always save the
            weak_ptrs and only upcast when needed. */
		boost::weak_ptr<QtUI::UICanvas> GetCanvasByID(const QString& id);

        /// @return The internal list of all canvases. Do not misuse!
        const QList<boost::shared_ptr<UICanvas> > &GetCanvases() const { return canvases_; }
             
        /** Hides the given canvas, also removing all mouse and keyboard focus from it. Prefer this
            to the canvas->Hide version. Do not pass in 0. */
        void HideCanvas(UICanvas &canvas);

        /**
         * Sets a current modifier. 
         * @param modifier is a current modifier.
         */
        void SetCurrentModifier(const Qt::KeyboardModifier& modifier) { currentModifier_ = modifier; }

        /**
         * Sets a active mouse button.
         * @param button is a active mousebutton. 
         */
        void SetActiveMouseButton(const Qt::MouseButton& button) { button_ = button; }



    public slots:
        /// Brings the canvas with the given id to topmost in the window drawing order.
        void SetTop(const QString& id);

        /// Pushes the canvas with the given id farthers in the window drawing order.
        void SetBack(const QString& id);

    signals:        
        /** This signal is emitted whenever the main render window size has changed.
            @param size The size of the new render window, in pixels. */
        void RenderWindowSizeChanged(const QSize& size);

    private:
        friend class QtModule;

        /** Evaluates the given mouse movement over the 2D main window and propagates that forward to the canvases.
            @param x The new mouse x-coordinate in the main render window, in pixels.
            @param y The new y-coordinate. Window top size is 0 and the coordinates progress downwards.
            @param deltaX The delta of the X coordinate in pixels since the last position of the mouse (last call to this function).
            @param deltaY The delta of the Y coordinate. */
        void InjectMouseMove(int x, int y, int deltaX, int deltaY, UICanvas* canvas);
     
        /** Sends a mouse button press event to correct canvas.
            @param x The x-coordinate in the main render window where the click occurred.
            @param y The y-coordinate.
            @note UIController expects that a corresponding mouse release message will be sent afterwards. */
        void InjectMousePress(int x, int y, UICanvas* canvas);
    

        /** Sends a mouse release event to correct canvas. 
            @param x The x-coordinate in the main render window where the mouse was when the mouse was released.
            @param y The y-coordinate. */
        void InjectMouseRelease(int x, int y, UICanvas* canvas);

        /** Sends a mouse double click event to correct canvas. 
            @param x The x-coordinate in the main render window where the double click occurred.
            @param y The y-coordinate.
            For the double click, there is no corresponding mouse release message that needs to be sent. */
        void InjectDoubleClick(int x, int y, UICanvas* canvas);

        /** Sends a key-press event to the canvas that currently has the active keyboard focus.
            @param text The textual representation of the key that was pressed.
            @param keyCode A Qt key code corresponding to that same key.
            @param modifier Denotes whether there were any keyboard modifiers down (Alt, Shift, Ctrl, Win).
            @note UIController expects that a corresponding key-release event is sent afterwards. */
        void InjectKeyPressed(QString& text, Qt::Key keyCode = Qt::Key_unknown, const Qt::KeyboardModifiers& modifier = Qt::NoModifier);

        /** Sends a key-release event to the canvas that received the previous key-press event.
            @param text The textual representation of the key that was pressed.
            @param keyCode A Qt key code corresponding to that same key.
            @param modifier Denotes whether there were any keyboard modifiers down (Alt, Shift, Ctrl, Win). */
        void InjectKeyReleased(const QString& text, Qt::Key keyCode = Qt::Key_unknown , const Qt::KeyboardModifiers& modifier = Qt::NoModifier);
      
        /**
         * Sends a mouse scroll event to the canvas. 
         * @param x is The x-coordinate in the main render window where scroll occured. 
         * @param t The y-coordinate
         * @param delta is a size of a scroll in degrees divided with Qt wheel step length. 
         * @param canvas is the canvas which will receive event. 
         */
        void InjectMouseScroll(int x, int y, int delta, UICanvas* canvas);


        /** Call to specify the new size of the main render window into which the canvases are composited.
            @param size The new size of the main render window, in pixels. */
        void SetParentWindowSize(const QSize& size);

        /** Runs the render logic of all canvases and manages other time-dependent animation related logic. 
            \todo Pass in frame deltatime to be able to properly compute the animations. */
        void Update();

        /** Sets the given canvas to receive the keyboard focus. If this focus is a result of a mouse click,
            pass in the click position (x,y) in main render window coordinates to have a widget in the canvas
            receive the keyboard focus as well (instead of just the canvas having it). */
        void ActivateKeyboardFocus(UICanvas *canvas, int x, int y);

        /** Refreshes the mouse cursor to show what the widget below the mouse cursor requests.
            @param canvas The canvas that currently has the mouse hover active, i.e. the canvas below the 
                mouse cursor, or 0 if there is no canvas below the mouse.
            @param x The mouse x-coordinate in the the main render window.
            @param y The y-coordinate. */
        void UpdateMouseCursor(UICanvas *canvas, int x, int y);

        /** @return The topmost visible UICanvas that contains the given point, which is passed in main 
            render window coordinates, or 0 if no such canvas exists. */
        UICanvas *GetCanvasAt(int x, int y);
       

        /** @return The index (to the internal list of canvases) of the topmost visible canvas that 
            contains the given point in render window coordinates, or -1 if no such canvas exists. */
        int GetCanvasIndexAt(const QPoint& point);

        /** @return The index (to the internal list of canvases) of the canvas with the given ID,
            or -1 if no such canvas exists. */
        int GetCanvasIndexByID(const QString& id) const;

        /** Recomputes the Z-values used to render the Ogre overlays, and maintains the invariant that
            always-on-top canvases are first on the list. Calling Arrange() invalidates all existing
            canvas indices received by calls to functions GetCanvasIndexAt and GetCanvasIndexByID. */
        void Arrange();

        /** Sends a mouse move event to the given canvas.
            @param x The mouse x-coordinate in the main render window coordinates. 
            @param y The y-coordinate. */
        void SendMouseMoveEvent(UICanvas &canvas, int x, int y);

        /** Sends a mouse button press event to the given canvas.
            @param x The mouse x-coordinate in the main render window coordinates where the press occurred.
            @param y The y-coordinate. */
        void SendMouseButtonPressEvent(UICanvas &canvas, int x, int y);

        void Redraw(const boost::shared_ptr<UICanvas>& canvas);

        enum DeactivationType { MouseMove, MousePress, MouseRelease, All };

        /** Sends the necessary mouse release and/or move messages to clear any hover activation visuals
            from the given canvas. */
        void Deactivate(UICanvas &canvas, DeactivationType type = All);

        /// Defines the possible mouse button actions that might be going on.
        enum MouseAction
        {
            /// There is no current mouse action being performed.
            MouseActionNone,

            /// The user is dragging a canvas from its title bar.
            MouseActionCanvasMove,  

            // The user is resizing the canvas from some direction.
            MouseActionCanvasResizeTopLeft, 
            MouseActionCanvasResizeTop,
            MouseActionCanvasResizeTopRight,
            MouseActionCanvasResizeLeft,
            MouseActionCanvasResizeRight,
            MouseActionCanvasResizeBottomLeft,
            MouseActionCanvasResizeBottom,
            MouseActionCanvasResizeBottomRight,

            /// The user is performing a mouse action inside the canvas - the action is just passed on to Qt.
            MouseActionCanvasInternal,
        };

        /** This function examines the given coordinates over the passed canvas and returns whether the coordinates
            lie in some resize or drag hotspot area.
            @return An element of the MouseAction enum that specifies which of the hotpot areas the given coordinates
                satisfy, or MouseActionNone if the coordinates do not lie in any hotspot. */
        MouseAction DetectCanvasHotSpot(UICanvas &canvas, int x, int y);

        /** The list of canvases owned by this UIController. The topmost canvas (in the drawing order) is stored
            first, and the bottommost is at the back. */
        QList<boost::shared_ptr<UICanvas> > canvases_;
        
        /// The location on the QGraphicsScene of the canvas where mouse mouse click was last time pressed.
        /// @note that This is not in main window render coordinates.
        QPoint lastMousePressPoint_;

        /// Specifies what kind of mouse action is currently being performed.
        MouseAction currentMouseAction;

        /** If currentMouseAction != None, specifies which UICanvas is the target canvas for that action.
            There may only be one canvas that is the recipient of a mouse action at any time, and this canvas
            steals all mouse input until the action is finished (mouse is released). This is 0 when there is 
            no mouse action going on. */
        UICanvas *mouseActionCanvas;

        /** Specifies the canvas that was mouse-hover active during the previous frame. This is stored here
            so that we can properly track when the currently hover-active canvas changes and any possible old
            visuals need to be cleared from the old canvas. This is 0 when the mouse is not on top of any canvas. */
        UICanvas *mouseHoverCanvas;

        /// This is the canvas that currently has all keyboard focus. If 0, no canvas has the keyboard focus.
        UICanvas *keyboardFocusCanvas;

        /** Tells the last widget we know that received keyboard focus. Used to force a clearing of the focus once the
            user clicks outside a canvas. Otherwise we could have several canvases have the keyboard focus simultaneously. */ 
        QGraphicsItem *lastKnownKeyboardFocusItem;

        /** The size of the main render window. Needed to be able to compute proper normalized [0,1]-sizes and coordinates
            for positioning Ogre overlays. */ 
        QSize parentWindowSize_;

        /// Tracks time elapsed since mouse click presses - used to detect double click events.
        /// \todo Move away from here to OIS.
        QTime doubleClickTimer_;

        /// Time which defines how soon two mouse press down events are assumed as double click.
        /// \todo Move away from here to OIS.
        int responseTimeLimit_;

        Qt::KeyboardModifier currentModifier_;
        Qt::MouseButton button_;
        
    };
}

#endif 
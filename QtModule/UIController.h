// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_QtModule_UIController_h
#define incl_QtModule_UIController_h

#include "UICanvas.h"

#include <QList>
#include <QPoint>
#include <QTime>
#include <QGraphicsSceneMouseEvent> 
#include <QKeyEvent>

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

namespace QtUI
{
    /**
     * UIController-class is used to manage OIS inputs and other window events which effect to UICanvases state. 
     * This class is a internal class which should not be used outside of QtModule. 
     */
    class UIController : public QObject
    {   
        Q_OBJECT
       
    
    public:
        UIController();
        virtual ~UIController();

        /**
         * Registers canvas to controller. 
         * @param canvas is canvas which state we want to manage through controller. 
         */

        void AddCanvas(UICanvas* canvas) {}
        
        /**
         * Draws/Updates canvases, after update canvases internal Z-order is calculated. 
         * @note this update only draws those canvases which has gone to dirty state. 
         */
        
        void Update();

        /**
         * Creates a new UICanvas which is managed by controller.
         * @param mode is canvas mode @see UICanvas 
         * @note default mode is Internal (so it is rendered by OGRE).
         */
        boost::weak_ptr<UICanvas> CreateCanvas(UICanvas::Mode mode = UICanvas::Internal);

        /**
         * Sends a mouse move event to correct canvas. 
         * @param x is a event position on x-axis. 
         * @param y is a event position on y-axis.
         * @note This function is for both hover and drag messages. 
         **/
        void InjectMouseMove(int x, int y);

     
        /** 
         * Sends a mouse press event to correct canvas.
         * @param x is a event position on x-axis. 
         * @param y is a event position on y-axis. 
         * @note A corresponing mouse release needs to be sent afterwards. 
         */
        void InjectMousePress(int x, int y);

    
        /**
         * Sends a mouse release event to correct canvas. 
         * @param x is a event position on x-axis.
         * @param y is a event position on y-axis. 
         */
        void InjectMouseRelease(int x, int y);

        /**
         * Sends a mouse double click event to correct canvas. 
         * @param x is a event position on x-axis. 
         * @param y is a event position on y-axis.
         */
        void InjectDoubleClick(int x, int y);

        /**
         * Sends a key pressed event to correct canvas. 
         * @param text is a key which was pressed (letter). 
         * @param keyCode is a code of key
         * @param modifier is a value which defines that was alt or shift down. 
         */

        void InjectKeyPressed(const QString& text, Qt::Key keyCode = Qt::Key_unknown, const Qt::KeyboardModifiers& modifier = Qt::NoModifier);

        /**
         * Sends a key released event to correct canvas. 
         * @param text is a key which was pressed (letter). 
         * @param keyCode is a code of key
         * @param modifier is a value which defines that was alt or shift down. 
         */

        void InjectKeyReleased(const QString& text, Qt::Key keyCode = Qt::Key_unknown , const Qt::KeyboardModifiers& modifier = Qt::NoModifier);
      
        /**
         * Sets a new size of Ogre-render window. 
         * @param size is new size of Ogre-render window. 
         */

        void SetParentWindowSize(const QSize& size);
        QSize GetParentWindowSize() const { return parentWindowSize_;}


        /**
         * Removes canvas from internal list. 
         * @param id is a canvas id. 
         */
        void RemoveCanvas(const QString& id);

        /**
         * Returns information about that should OIS keyboard to be in buffered state or not. 
         * @return true if keyboard should set to bufferd false if not.
         */
        bool IsKeyboardFocus() const { return keyboard_buffered_; }


        /**
         * Updates mouse cursor to correct state. Returns current shape of mouse cursor.
         * @param x is x-axis coordinate of cursor position in render window coordinate system.
         * @param y is y-axis coordiante of cursor position in render window coordinate system.
         * @param index is index to canvas which is below of mouse cursor.
         */

        Qt::CursorShape UpdateMouseCursor(int x, int y, int index = -1);

        /**
         * Returns a canvas from a given point. 
         * @param x is a x-coordinate of canvas. 
         * @param y is a y-coordinate of canvas. 
         */
        
        UICanvas* GetCanvasAt(int x, int y);
		

		/**
         * Returns a canvas with the given id.
		 * @param id is the id of the canvas
		 */
		boost::shared_ptr<QtUI::UICanvas> GetSharedCanvasPtr(const QString& id);

    public slots:
        
        

        /** 
         * Sets canvas to front of all canvases. 
         * @param id is a canvas id.
         */

        void SetTop(const QString& id);
        
        /**
         * Sets canvas to back of all other canvases.
         * @param id is a canvas id.
         */

        void SetBack(const QString& id);

    signals:
        
        /**
         * Signal which is emited when Ogre render window has size has changed. 
         * @param size is a new size of window. 
         */
        void RenderWindowSizeChanged(const QSize& size);

    protected:
        
        /**
         * Returns index to canvas which is on given position. 
         * if there does not exist any it will negative index -1. 
         * @return index of canvas if there is any it will return negative index -1
         */
        virtual int GetCanvas(const QPoint& point);
    
       

    private:

        /**
         * Arrange canvases to so that first canvas in list has highest Z-value, meaning that it is top of everything.
         * @note arrange is done only if some of canvases has request a arrange. 
         */
        void Arrange();

        /**
         * Searches canvas which id is given. 
         * @param id is id of canvas which is searched.
         */
        int Search(const QString& id) const;

        /**
         * Checks that does given canvas contain given point. 
         * @param canvas (assumed rectangle geometry).
         * @param point is point which we want to check.
         */
        bool Contains(const boost::shared_ptr<UICanvas>& canvas, const QPoint& point) const;


        /**
         * Helper function which sends a mouse move event to given canvas. 
         * @param index is index to canvas. 
         * @param x is x-axis coordinate in render window coordinate frame. 
         * @param y is y-axis coordinate in render window coordinate frame.
         */

        void SendMouseMoveEvent(int index, int x, int y);

        // Assumed to be in that kind order that top (active) canvas is first.
        QList<boost::shared_ptr<UICanvas> > canvases_;
        
        // Is mouse currently down?
        bool mouseDown_;
       
        // Last location of mouse press down.
        QPoint mousePress_;

        // Is Z-order changed? Should it arrange again.
        //bool arrange_;

        // Render window size.
        QSize parentWindowSize_;

        // Last mouse position.
        QPoint lastPosition_;

        // Timer which is used to detect double click events. 
        QTime timer_;

        // Time which defines how soon two mouse press down events are assumed as double click.  
        int responseTimeLimit_;

        // Is some key hold down. 
        bool keyDown_;

        // Last key event. 
        QKeyEvent lastKeyEvent_;

        QTime keyTimer_;
        
        // Time when key press down event is though as a multiple key press event. 
        int multipleKeyLimit_;

        // Current pressed keys
        QList<QPair<Qt::Key, QString> > pressedKeys_;

        // Should keyboard to put buffered state?
        bool keyboard_buffered_;

        // Currently (or last) active canvas id.
        QString active_canvas_;

        // Current mouse cursor shape. (note is not always valid!)
        Qt::CursorShape mouseCursorShape_;

        bool resize_;

        bool drag_;
    };


}

#endif 
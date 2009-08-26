// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_QtModule_UIController_h
#define incl_QtModule_UIController_h

#include "UICanvas.h"

#include <QList>
#include <QPoint>
#include <QGraphicsSceneMouseEvent> 

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

namespace QtUI
{
    /**
     * UIController-class is used to manage OIS inputs and other window events which effect to UICanvases state. 
     *
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

    public slots:
        
        /**
         * Reques arrange of canvases to new Z-order. 
         */
        void RequestArrange() { arrange_ = true; }

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
    
        /**
         * Arrange canvases to Z-order.
         * @note arrange is done only if some of canvases has request a arrange. 
         */
        virtual void Arrange();

    private:

        /**
         * Checks that does given canvas contain given point. 
         * @param canvas (assumed rectangle geometry).
         * @param point is point which we want to check.
         */
        bool Contains(const boost::shared_ptr<UICanvas>& canvas, const QPoint& point) const;

        // Assumed to be in Z-order. 
        QList<boost::shared_ptr<UICanvas> > canvases_;
        
        // Is mouse currently down?
        bool mouseDown_;

        // Last location of mousePress down.
        QPoint mousePress_;
       
        // Is Z-order changed? Should it arrange again.
        bool arrange_;

        // Render window size.
        QSize parentWindowSize_;

    };


}

#endif 
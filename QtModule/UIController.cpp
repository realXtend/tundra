
// For conditions of distribution and use, see copyright notice in license.txt

#include "UIController.h"
#include <QApplication>
#include <QDebug>
#include <cmath>
#include <QKeyEvent>

#include "MemoryLeakCheck.h"

namespace QtUI
{

UIController::UIController() : mouseDown_(false), arrange_(false), responseTimeLimit_(500)
{}

UIController::~UIController()
{}

void UIController::Update()
{

    QList<boost::shared_ptr<UICanvas> >::iterator iter = canvases_.begin();
    for(; iter != canvases_.end(); ++iter)
        (*iter)->Render();
    
    //Update Z-order if needed.

    Arrange();    
}

void UIController::Arrange()
{
    if (arrange_)
    {
        ///todo Arrange canvases to z-order.
    }
    arrange_ = false;
        
}

void UIController::RemoveCanvas(const QString& id)
{
    QList<boost::shared_ptr<UICanvas> >::iterator iter = canvases_.begin();
    int index = 0;
    for(; iter != canvases_.end(); ++iter, ++index)
    {
        QString canvas_id = (*iter)->GetID();
        if ( canvas_id == id)
        {
            canvases_.removeAt(index);
            break;
        }
    }
}


boost::weak_ptr<UICanvas> UIController::CreateCanvas(UICanvas::Mode mode)
{
    boost::shared_ptr<UICanvas> canvas(new UICanvas(mode, parentWindowSize_));
    
    QObject::connect(this,SIGNAL(RenderWindowSizeChanged(const QSize&)),canvas.get(), SLOT(SetRenderWindowSize(const QSize&)));
    QObject::connect(canvas.get(), SIGNAL(RequestArrange()), this, SLOT(RequestArrange()));

    canvases_.append(canvas);
    return canvas;
}

void UIController::InjectMouseMove(int x, int y)
{
   
    QPoint point(x,y);
    int index = GetCanvas(point);
    
    // Normal move. 

    if (index != -1 && !mouseDown_)
    {
 
        // Location of mouse event in scene.
        QPoint p = canvases_[index]->MapToCanvas(x,y);
        QPointF pos = canvases_[index]->mapToScene(p);
        
        //QPointF pos = canvases_[index]->mapToScene(point);
        
        QPoint currentMousePos((int)pos.x(), (int)pos.y());

        QGraphicsSceneMouseEvent mouseEvent(QEvent::GraphicsSceneMouseMove);
        
        if (mouseDown_)
        {
            mouseEvent.setButtonDownScenePos(Qt::LeftButton, mousePress_);
            mouseEvent.setButtonDownScreenPos(Qt::LeftButton, mousePress_);
        }
        else
        {
            mouseEvent.setButtonDownScenePos(Qt::NoButton, currentMousePos);
            mouseEvent.setButtonDownScreenPos(Qt::NoButton, currentMousePos);
        }
        
        mouseEvent.setScenePos(currentMousePos);
        mouseEvent.setScreenPos(currentMousePos);
        mouseEvent.setLastScenePos(currentMousePos);
        mouseEvent.setLastScreenPos(currentMousePos);
        mouseEvent.setButtons(mouseDown_ ? Qt::LeftButton : Qt::NoButton);
        mouseEvent.setButton(mouseDown_ ? Qt::LeftButton : Qt::NoButton);
        mouseEvent.setModifiers(0);
        mouseEvent.setAccepted(false);
    
        QApplication::sendEvent(canvases_[index]->scene(), &mouseEvent);    
        
    }
    else if ( mouseDown_ )
    {
        // Drag event. 
        
        // Find active canvas, move it to this location (if canvas is not locked.)
        
        index = GetCanvas(lastPosition_);
        
        if ( index != -1)
        {
            if ( !canvases_[index]->IsCanvasPositionLocked() )
            {
                QPoint pos = canvases_[index]->GetPosition().toPoint();

                int xPos = point.x()-(lastPosition_.x()-pos.x());
                int yPos = point.y()-(lastPosition_.y()-pos.y());
              
                canvases_[index]->SetPosition(xPos, yPos);
            }
        }

    }
    
    lastPosition_ = point;

}

void UIController::InjectMousePress(int x, int y)
{
  
    QPoint point(x,y);
  
    int index = GetCanvas(point);
    
    if ( timer_.isNull() )
    {
        // First press. 
        timer_.start();
    }
    else if ( timer_.elapsed() <= responseTimeLimit_)
    {
        // Double click has happen.
       
        // Restart to zero.
        timer_ = QTime();
        
        QPoint loc = lastPosition_ - point;
        lastPosition_ = point;
        if ( loc.manhattanLength() < 2)
        {
            InjectDoubleClick(x,y);
            return;
        }
      
        
    }
    else 
    {
        // Restart to zero.
        timer_ = QTime();
        // And start again.    
        timer_.start();
    }
    
    
    if (index != -1)
    {
    
        // Translate the mouse position from QGraphicsView coordinate frame onto
        // the QGraphicsScene coordinate frame.
        QPoint p = canvases_[index]->MapToCanvas(x,y);
        QPointF pos = canvases_[index]->mapToScene(p);
        QPoint currentMousePos((int)pos.x(), (int)pos.y());

        // For future use save press state. 
        mouseDown_ = true;
        mousePress_ = point;

        QGraphicsSceneMouseEvent mouseEvent(QEvent::GraphicsSceneMousePress);
        
        mouseEvent.setButtonDownScenePos(Qt::LeftButton, currentMousePos);
        mouseEvent.setButtonDownScreenPos(Qt::LeftButton, currentMousePos);
        mouseEvent.setScenePos(currentMousePos);
        mouseEvent.setScreenPos(currentMousePos);
        mouseEvent.setLastScenePos(currentMousePos);
        mouseEvent.setLastScreenPos(currentMousePos);
        mouseEvent.setButtons(Qt::LeftButton);
        mouseEvent.setButton(Qt::LeftButton);

        mouseEvent.setModifiers(0);
        mouseEvent.setAccepted(false);
        
        QApplication::sendEvent(canvases_[index]->scene(), &mouseEvent);

        
    }
    lastPosition_ = point;
}

void UIController::InjectMouseRelease(int x, int y)
{
    ////todo what to do release after double click?
  
    QPoint point(x,y);
    int index = GetCanvas(point);

    if (index != -1)
    {
        
        // Translate the mouse position from QGraphicsView coordinate frame onto
        // the QGraphicsScene coordinate frame.
        QPoint p = canvases_[index]->MapToCanvas(x,y);
        QPointF pos = canvases_[index]->mapToScene(p);
        QPoint currentMousePos((int)pos.x(), (int)pos.y());

        QGraphicsSceneMouseEvent mouseEvent(QEvent::GraphicsSceneMouseRelease);
        mouseEvent.setButtonDownScenePos(Qt::NoButton, currentMousePos);
        mouseEvent.setButtonDownScreenPos(Qt::NoButton, currentMousePos);
        mouseEvent.setScenePos(currentMousePos);
        mouseEvent.setScreenPos(currentMousePos);
        mouseEvent.setLastScenePos(currentMousePos);
        mouseEvent.setLastScreenPos(currentMousePos);
        mouseEvent.setButtons(Qt::NoButton);
        mouseEvent.setButton(Qt::LeftButton);
        mouseEvent.setModifiers(0);
        mouseEvent.setAccepted(false);
     
       

        QApplication::sendEvent(canvases_[index]->scene(), &mouseEvent);
    
    }

    mouseDown_ = false;
    lastPosition_ = point;
}

void UIController::InjectDoubleClick(int x, int y)
{
    QPoint point(x,y);
    lastPosition_ = point;
    int index = GetCanvas(point);
    
    if (index != -1)
    {
    
        // Translate the mouse position from QGraphicsView coordinate frame onto
        // the QGraphicsScene coordinate frame.
        QPoint p = canvases_[index]->MapToCanvas(x,y);
        QPointF pos = canvases_[index]->mapToScene(p);
        QPoint currentMousePos((int)pos.x(), (int)pos.y());

        mouseDown_ = false;
        
        // In case of double click we set that left button generated click -> so it is set down. 

        QGraphicsSceneMouseEvent mouseEvent(QEvent::GraphicsSceneMouseDoubleClick);

        mouseEvent.setButtonDownScenePos(Qt::LeftButton, currentMousePos);
        mouseEvent.setButtonDownScreenPos(Qt::LeftButton, currentMousePos);
        mouseEvent.setScenePos(currentMousePos);
        mouseEvent.setScreenPos(currentMousePos);
        mouseEvent.setLastScenePos(currentMousePos);
        mouseEvent.setLastScreenPos(currentMousePos);
        mouseEvent.setButtons(Qt::LeftButton);
        mouseEvent.setButton(Qt::LeftButton);
 
        mouseEvent.setModifiers(0);
        mouseEvent.setAccepted(false);
     
        QApplication::sendEvent(canvases_[index]->scene(), &mouseEvent);
    
    }
 
}

void UIController::InjectKeyPressed(Qt::Key keyCode, const Qt::KeyboardModifier& modifier)
{
    QKeySequence sequence(keyCode);  
    QKeyEvent keyEvent(QEvent::KeyPress, keyCode, modifier, sequence.toString().toLower());
    keyEvent.setAccepted(false);
    
    // Take a location of last known mouse press and send it to that canvas. 
    
    int index = GetCanvas(mousePress_);
    if ( index != -1 )
    {
         QApplication::sendEvent(canvases_[index]->scene(), &keyEvent);
    }

}

void UIController::SetParentWindowSize(const QSize& size)
{
    parentWindowSize_ = size;
    emit RenderWindowSizeChanged(size);
}


int UIController::GetCanvas(const QPoint& point) 
{
    QList<boost::shared_ptr<UICanvas> >::iterator iter = canvases_.begin();
    int index = 0;
    for (; iter != canvases_.end(); ++iter, ++index)
    {
        UICanvas::Mode mode = (*iter)->GetMode();
        
        switch(mode)
        {
            // If mode is external we let the Qt own window manager to things. 
            case UICanvas::External:
                break;
            case UICanvas::Internal:
                {
                   if (Contains(*iter, point))
                        return index;
                    break;
                }
           
            default:
                break;  

        }

    }

    // Did not find any canvases so let's return -1.

    return -1;
}

bool UIController::Contains(const boost::shared_ptr<UICanvas>& canvas, const QPoint& point) const
{
    QPoint pos = canvas->GetPosition().toPoint();
    return pos.x() <= point.x() && 
           pos.y() <= point.y() && 
           (pos.x() + canvas->width()) > point.x() && 
           (pos.y() + canvas->height()) > point.y();
}

}
        
     
     
   

      
       

     
      
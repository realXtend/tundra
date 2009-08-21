
// For conditions of distribution and use, see copyright notice in license.txt

#include "UIController.h"
#include <QApplication>

namespace QtUI
{

UIController::UIController() 
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


boost::weak_ptr<UICanvas> UIController::CreateCanvas(UICanvas::Mode mode)
{
    boost::shared_ptr<UICanvas> canvas(new UICanvas(mode));
    canvas->SetRenderWindowSize(parentWindowSize_);
    
    QObject::connect(this,SIGNAL(RenderWindowSizeChanged(const QSize&)),canvas.get(), SLOT(SetRenderWindowSize(const QSize&)));
    QObject::connect(canvas.get(), SIGNAL(RequestArrange()), this, SLOT(RequestArrange()));

    canvases_.append(canvas);
    return canvas;
}

void UIController::InjectMouseMove(int x, int y)
{
    QPoint point(x,y);
    
    int index = GetCanvas(point);
    
    if (index != -1)
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
}

void UIController::InjectMousePress(int x, int y)
{
    QPoint point(x,y);
    
    int index = GetCanvas(point);
    
    if (index != -1)
    {
    
        // Translate the mouse position from QGraphicsView coordinate frame onto
        // the QGraphicsScene coordinate frame.
        QPoint p = canvases_[index]->MapToCanvas(x,y);
        QPointF pos = canvases_[index]->mapToScene(p);
        QPoint currentMousePos((int)pos.x(), (int)pos.y());

        // For future use save press state. 
        mouseDown_ = true;
        mousePress_ = currentMousePos;

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
}

void UIController::InjectMouseRelease(int x, int y)
{
    QPoint point(x,y);
    
    int index = GetCanvas(point);
    
    if (index != -1)
    {
    
        // Translate the mouse position from QGraphicsView coordinate frame onto
        // the QGraphicsScene coordinate frame.
        QPoint p = canvases_[index]->MapToCanvas(x,y);
        QPointF pos = canvases_[index]->mapToScene(p);
        QPoint currentMousePos((int)pos.x(), (int)pos.y());

        mouseDown_ = false;

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
            case UICanvas::Embedded:
                {
                    // Currently we don't know how to deal 3D-canvases. 
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
           (pos.x() + canvas->width()) >= point.x() && 
           (pos.y() + canvas->height()) >= point.y();
}

}
        
     
     
   

      
       

     
      
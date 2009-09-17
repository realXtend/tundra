// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"

#include "DebugOperatorNew.h"

#include "UIController.h"
#include <QApplication>
#include <QDebug>
#include <cmath>
#include <QKeyEvent>
#include <QGraphicsItem>
#include <QGraphicsWidget>
#include "MemoryLeakCheck.h"

namespace QtUI
{

UIController::UIController() : mouseDown_(false), arrange_(false), responseTimeLimit_(500), keyDown_(false), lastKeyEvent_(QKeyEvent(QEvent::KeyPress,0,Qt::NoModifier)), multipleKeyLimit_(150), keyboard_buffered_(false), active_canvas_("")
{}

UIController::~UIController()
{}

void UIController::Update()
{
    
    if ( keyDown_ && keyTimer_.elapsed() > multipleKeyLimit_ )
    {
        //HACK
        keyTimer_ = QTime();
        keyTimer_.start();
    
        for ( QList<QPair<Qt::Key, QString> >::iterator iter = pressedKeys_.begin(); iter != pressedKeys_.end(); ++iter)
            InjectKeyPressed((*iter).second, (*iter).first);
        

    }
    

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
        /// Arrange canvases to somekind Z-order, currently just put latest active canvas to first in list. 
        
        for ( int i = 0; i < canvases_.size(); ++i)
        {
            if ( canvases_[i]->GetID() == active_canvas_)
            {
                boost::shared_ptr<UICanvas> canvas = canvases_.takeAt(i);
                canvases_.prepend(canvas);
                break;
            }
        }
       

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
    QObject::connect(canvas.get(), SIGNAL(RequestArrange(const QString&)), this, SLOT(RequestArrange(const QString&)));

    canvases_.append(canvas);
    return canvas;
}

void UIController::InjectMouseMove(int x, int y)
{
   
    QPoint point(x,y);
    int index = GetCanvas(point);
    
    // Normal move. 

    if (index != -1 && !mouseDown_ && !canvases_[index]->IsHidden())
    {
 
        // Location of mouse event in scene.
        QPoint p = canvases_[index]->MapToCanvas(x,y);
        QPointF pos = canvases_[index]->mapToScene(p);
          
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
    
       
        UpdateMouseCursor(x,y,index);
        QApplication::sendEvent(canvases_[index]->scene(), &mouseEvent);   
        
        
       
  
    }
    else if ( mouseDown_)
    {
        // Drag event. 
        
        // Find active canvas, move it to this location (if canvas is not locked.)
        
        index = GetCanvas(lastPosition_);
        
        if ( index != -1 && !canvases_[index]->IsHidden())
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
    keyboard_buffered_ = false;

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
    
    mouseDown_ = true;

    if (index != -1 && !canvases_[index]->IsHidden())
    {
    
        // Translate the mouse position from QGraphicsView coordinate frame onto
        // the QGraphicsScene coordinate frame.
        QPoint p = canvases_[index]->MapToCanvas(x,y);
        QPointF pos = canvases_[index]->mapToScene(p);
        QPoint currentMousePos((int)pos.x(), (int)pos.y());

        // For future use save press state. 
      
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
        
        // Change last active canvas back to normal state.
        if ( active_canvas_ != "")
        {
            for ( int i = 0; i < canvases_.size(); ++i)
                if ( canvases_[i]->GetID() == active_canvas_)
                {
                    // Z -order value 1 is normal value.
                    canvases_[i]->SetZOrder(1);
                    break;
                }
                
        }

        // Change new canvas to a active overlay.
        canvases_[index]->SetZOrder(2);
        // Request change our internal "order" 
        RequestArrange(canvases_[index]->GetID());

        QApplication::sendEvent(canvases_[index]->scene(), &mouseEvent);
        
        // Here starts nice HACK: Idea is to check that did press event went to somekind textedit widget. 
        // if it went we need to set OIS keyboard to buffered mode. 

        QGraphicsItem* item = canvases_[index]->itemAt(mouseEvent.pos().toPoint());
        if ( item != 0)
        {
            QGraphicsWidget* widget = item->topLevelWidget();
            
            if ( widget->hasCursor() )
            {            
                QCursor cursor = widget->cursor();
                Qt::CursorShape shape = cursor.shape();
                if ( shape == Qt::IBeamCursor)
                    keyboard_buffered_ = true;
              
            }
            
        }

      
        QCursor cursor = canvases_[index]->cursor();
        Qt::CursorShape shape = cursor.shape();
        if ( shape == Qt::SizeBDiagCursor || shape == Qt::SizeHorCursor || shape == Qt::SizeVerCursor)
         {
                    qDebug()<<"We want to resize canvas!";
         }
       
        
    }
    lastPosition_ = point;
}

void UIController::InjectMouseRelease(int x, int y)
{
    ////todo what to do release after double click?
  
    QPoint point(x,y);
    int index = GetCanvas(point);

    if (index != -1 && !canvases_[index]->IsHidden())
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
    
    if (index != -1 && !canvases_[index]->IsHidden())
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

void UIController::InjectKeyPressed(const QString& text, Qt::Key keyCode, const Qt::KeyboardModifiers& modifier)
{
   
   
    QKeyEvent keyEvent(QEvent::KeyPress, keyCode, modifier, text);
    keyEvent.setAccepted(false);
    
    // Take a location of last known mouse press and send it to that canvas. 
    
    int index = GetCanvas(mousePress_);
    if ( index != -1 && !canvases_[index]->IsHidden())
    {
         QApplication::sendEvent(canvases_[index]->scene(), &keyEvent);
         keyDown_ = true;
         lastKeyEvent_ = keyEvent;
         keyTimer_.start();

         // Add key into list (if it is unique).
        
         if ( !pressedKeys_.contains(qMakePair(keyCode, text)))
              pressedKeys_.append(qMakePair(keyCode, text));
    }

}

void UIController::InjectKeyReleased(const QString& text, Qt::Key keyCode, const Qt::KeyboardModifiers& modifier)
{
 
    QKeySequence sequence(keyCode);  

    QKeyEvent keyEvent(QEvent::KeyRelease, keyCode, modifier, sequence.toString().toLower());
    keyEvent.setAccepted(false);
    
    // Take a location of last known mouse press and send it to that canvas. 
    
    int index = GetCanvas(mousePress_);
    if ( index != -1 && !canvases_[index]->IsHidden() )
    {
         QApplication::sendEvent(canvases_[index]->scene(), &keyEvent);
         keyTimer_ = QTime();
    }

  
    int size = pressedKeys_.size();
    for (int i = size; i--;)
    {
        if (pressedKeys_[i].first == keyCode)
            pressedKeys_.removeAt(i);
    }
    
   
    
    keyDown_ = false;
}

void UIController::SetParentWindowSize(const QSize& size)
{
    parentWindowSize_ = size;
    emit RenderWindowSizeChanged(size);
}

void UIController::UpdateMouseCursor(int x, int y, int index)
{
    if ( index != -1)
    {
        QRect frame = canvases_[index]->frameGeometry();
        QPointF pos = canvases_[index]->GetPosition();
        
        // Rectangular sides. 
        
        int bottom = pos.y() + frame.height();
        int bottomLeft = pos.x();
        int bottomRight = pos.x() + frame.width();
        int top = pos.y();
        
        int side_margin = 4;
        int corner_margin = 4;
        
        // Corners.
        QRect left_bottom_corner_box(QPoint(bottomLeft,bottom-corner_margin),QPoint(bottomLeft+corner_margin, bottom));
        QRect left_top_corner_box(QPoint(bottomLeft, top), QPoint(bottomLeft + corner_margin, top + corner_margin));
        
        QRect right_top_corner_box(QPoint(bottomRight, top), QPoint(bottomRight - corner_margin, top + corner_margin));
        QRect right_bottom_corner_box(QPoint(bottomRight - corner_margin, bottom - corner_margin), QPoint(bottomRight,bottom));
        
        // Sides

        QRect top_side_box(QPoint(bottomLeft + corner_margin, top), QPoint(bottomRight - corner_margin, top - side_margin));
        QRect bottom_side_box(QPoint(bottomLeft - corner_margin, bottom - side_margin), QPoint(bottomRight - corner_margin, bottom));

        QRect left_side_box(QPoint(bottomLeft, top + corner_margin), QPoint(bottomLeft + side_margin, bottom - corner_margin));
        QRect right_side_box(QPoint(bottomRight-corner_margin, top + side_margin), QPoint(bottomRight, bottom - corner_margin));

        QPoint point(x,y);

        if ( left_bottom_corner_box.contains(point) )
            QApplication::setOverrideCursor(QCursor(Qt::SizeBDiagCursor));
        else if ( left_top_corner_box.contains(point) )
            QApplication::setOverrideCursor(QCursor(Qt::SizeFDiagCursor));
        else if ( right_top_corner_box.contains(point))
            QApplication::setOverrideCursor(QCursor(Qt::SizeBDiagCursor));
        else if ( right_bottom_corner_box.contains(point) )
            QApplication::setOverrideCursor(QCursor(Qt::SizeFDiagCursor));
        else if (top_side_box.contains(point))
            QApplication::setOverrideCursor(QCursor(Qt::SizeVerCursor));
        else if (bottom_side_box.contains(point) )
            QApplication::setOverrideCursor(QCursor(Qt::SizeVerCursor));
        else if (left_side_box.contains(point))
            QApplication::setOverrideCursor(QCursor(Qt::SizeHorCursor));
        else if (right_side_box.contains(point))
            QApplication::setOverrideCursor(QCursor(Qt::SizeHorCursor));
        else
            QApplication::setOverrideCursor(QCursor(Qt::ArrowCursor));
            //QApplication::restoreOverrideCursor();   

    }

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
        
     
     
   

      
       

     
      
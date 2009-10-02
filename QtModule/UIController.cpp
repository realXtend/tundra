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

UIController::UIController() : mouseDown_(false), responseTimeLimit_(500), keyDown_(false), lastKeyEvent_(QKeyEvent(QEvent::KeyPress,0,Qt::NoModifier)), multipleKeyLimit_(150), keyboard_buffered_(false), active_canvas_(""), resize_(false)
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
    
   
}

int UIController::Search(const QString& id) const
{
    for ( int i = 0; i < canvases_.size(); ++i)
        if ( canvases_[i]->GetID() == id)
            return i;

    
    return -1;
   
        
}

void UIController::Arrange()
{
    int size = canvases_.size();
    for ( int i = 0; i < size; ++i)
        canvases_[i]->SetZOrder(size - i);

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
    QObject::connect(canvas.get(), SIGNAL(ToTop(const QString&)), this, SLOT(SetTop(const QString&)));
    QObject::connect(canvas.get(), SIGNAL(ToBack(const QString&)), this, SLOT(SetBack(const QString&)));

    // Adds automatically and "Z-order". So that last created canvas is top.
    canvas->SetZOrder(canvases_.size() + 1);
    canvases_.prepend(canvas);
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
        // Drag event or resize 
        
        // Find active canvas, move it to this location (if canvas is not locked.)
        
        index = GetCanvas(lastPosition_);
        
        if ( index != -1 && !canvases_[index]->IsHidden())
        {
        
            //if ( !resize_ && mouseCursorShape_ == Qt::SizeVerCursor || 
            //        mouseCursorShape_ == Qt::SizeHorCursor || 
            //        mouseCursorShape_ == Qt::SizeBDiagCursor || 
            //        mouseCursorShape_ == Qt::SizeFDiagCursor )
            //{
            //    resize_ = true;

                // Resize canvas.
                // 1. Pienentäminen, pitää muuttaa overlayin kokoa, ja sitten tekstuurin uv-mäppäystä, lopuksi luo uusi tekstuuri, joka on oikean kokoinen
                // ja sisältää (ei skaalatun) tekstuurin. Huom! tee boolean, joka estää resize toiminnallisuuden. 
                // 2. Kasvattaminen tuplaa tekstuurin koko, näytä osa siitä uv-mäppäyksellä. Lopuksi luo oikean kokoinen ja näytä se. 
             
            //}
            if ( resize_)
            {
                index = Search(active_canvas_);
                if (index == -1)
                    return;

                QRect geometry = canvases_[index]->GetCanvasGeometry();
                int width = geometry.width();
                int height = geometry.height();

                QPoint position = canvases_[index]->GetPosition().toPoint();

                if ( mouseCursorShape_ == Qt::SizeVerCursor)
                {
                    //height changes.
                    int bottom = position.y() + height;
                    int top = position.y();

                    // Now search nearest side 
                    
                    double bottomDiff = sqrt(double((bottom - y)* (bottom - y)));
                    double topDiff = sqrt(double((top - y) * (top - y)));
                    
                    if ( bottomDiff > topDiff)
                    {
                        // Top-side is resizing.
                        if ( top < y ) 
                        {
                            // Smaller
                            canvases_[index]->Resize(height-int(topDiff), width, UICanvas::Top);
                        }
                        else
                        {
                            // Growing
                         
                        }
                        
                    }
                    else
                    {
                        // Bottom-side is resizing.
                        canvases_[index]->Resize(height-int(bottomDiff), width, UICanvas::Bottom);
                    }

                }
                else if ( mouseCursorShape_ == Qt::SizeHorCursor)
                {
                    // width changes.
                    
                    int left = position.x();
                    int right = position.x() + width;

                    // Search nearest side
                    double leftDiff = sqrt(double((left - x)* (left - x)));
                    double rightDiff = sqrt(double((right - x) * (right - x)));
                    
                      if ( leftDiff > rightDiff)
                      {
                        // Right side.
                        canvases_[index]->Resize(height, width-int(rightDiff), UICanvas::Right);
                        
                      }
                      else
                      {
                        canvases_[index]->Resize(height, width-int(leftDiff), UICanvas::Left);                        
                         
                      }
        

                      

                }
                else if ( mouseCursorShape_ == Qt::SizeBDiagCursor)
                {
                   
                    // Corners

                    // Left-bottom. 
                    int left_bottom_x = position.x();
                    int left_bottom_y = position.y()+height;

                    // Right-top
                    int right_top_x = position.x() + width;
                    int right_top_y = position.y();

                    // Search nearest corner
                    double dist_right_corner = (right_top_x - x)*(right_top_x - x) + (right_top_y - y)*(right_top_y - y);
                    double dist_left_corner = (left_bottom_x - x)*(left_bottom_x - x) + (left_bottom_y - y)*(left_bottom_y - y);

                   
                    geometry.setRect(position.x(), position.y(), geometry.width(), geometry.height());
                    if ( dist_right_corner > dist_left_corner)
                    {
                         // Nearest corner is left corner.
                        if (geometry.contains(QPoint(x,y)))
                        {
                            // Making smaller.
                            int diff = x - position.x();
                            canvases_[index]->Resize(height, width-diff, UICanvas::Left);
                            diff = position.y() + height - y;
                            canvases_[index]->Resize(height-diff, canvases_[index]->GetSize().width(), UICanvas::Bottom);
                        }
                        else
                        {
                            // Growing.

                        }

                    }
                    else
                    {
                        // Nearest corner is right corner.
                        if (geometry.contains(QPoint(x,y)))
                        {
                            // Making smaller.
                            int diff = position.x() + width - x;
                            canvases_[index]->Resize(height, width-diff, UICanvas::Right);
                            diff = y - position.y();
                            canvases_[index]->Resize(height-diff, canvases_[index]->GetSize().width(), UICanvas::Top);
                        }
                        else
                        {
                            // Growing.

                        }

                    }
                  
                }
                else if ( mouseCursorShape_ == Qt::SizeFDiagCursor)
                {
                    // Corners

                    // Left-top. 
                    int left_top_x = position.x();
                    int left_top_y = position.y();

                    // Right-bottom
                    int right_bottom_x = position.x() + width;
                    int right_bottom_y = position.y() + height;

                    // Search nearest corner
                    double dist_left_corner = (left_top_x - x)*(left_top_x - x) + (left_top_y - y)*(left_top_y - y);
                    double dist_right_corner = (right_bottom_x - x)*(right_bottom_x - x) + (right_bottom_y - y)*(right_bottom_y - y);

                   
                    geometry.setRect(position.x(), position.y(), geometry.width(), geometry.height());
                    
                    if ( dist_right_corner > dist_left_corner)
                    {
                         // Nearest corner is left corner.
                        if (geometry.contains(QPoint(x,y)))
                        {
                            // Making smaller.
                            int diff = x - position.x();
                            canvases_[index]->Resize(height, width-diff, UICanvas::Left);
                            diff = y - position.y();
                            canvases_[index]->Resize(height-diff, canvases_[index]->GetSize().width(), UICanvas::Top);
                        }
                        else
                        {
                            // Growing.

                        }

                    }
                    else
                    {
                        // Nearest corner is right corner.
                        if (geometry.contains(QPoint(x,y)))
                        {
                            // Making smaller.
                            int diff = position.x() + width - x ;
                            canvases_[index]->Resize(height, width-diff, UICanvas::Right);
                            diff = position.y() + height - y;
                            canvases_[index]->Resize(height-diff, canvases_[index]->GetSize().width(), UICanvas::Bottom);
                        }
                        else
                        {
                            // Growing.

                        }

                    }


                }

              
                
                
                QApplication::setOverrideCursor(QCursor(mouseCursorShape_));
                
            }
            else if (!canvases_[index]->IsCanvasPositionLocked() )
            {
                // Drag canvas. 

                QPoint pos = canvases_[index]->GetPosition().toPoint();

                int xPos = point.x()-(lastPosition_.x()-pos.x());
                int yPos = point.y()-(lastPosition_.y()-pos.y());
              
                canvases_[index]->SetPosition(xPos, yPos);
            }
          
        }

    }
    
    lastPosition_ = point;

}

void UIController::SetTop(const QString& id ) 
{
    int index = Search(id);
    if ( index == -1)
        return;

    boost::shared_ptr<UICanvas> canvas = canvases_.takeAt(index);
    canvases_.prepend(canvas);

    // Assure that it is current active canvas
    active_canvas_ = id;
    
    Arrange();
    

}

void UIController::SetBack(const QString& id)
{
    int index = Search(id);
    if ( index == -1)
        return;

    boost::shared_ptr<UICanvas> canvas = canvases_.takeAt(index);
    canvases_.append(canvas);

    active_canvas_ = "";
    Arrange();

}


void UIController::InjectMousePress(int x, int y)
{
    
    QPoint point(x,y);
    keyboard_buffered_ = false;

    int index = GetCanvas(point);
    
    UpdateMouseCursor(x,y,index);
    
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
        /*
        if ( active_canvas_ != "")
        {
            SetBack(active_canvas_);

        }
        */
       
       
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
        
        if ( mouseCursorShape_ == Qt::SizeVerCursor || 
             mouseCursorShape_ == Qt::SizeHorCursor || 
             mouseCursorShape_ == Qt::SizeBDiagCursor || 
             mouseCursorShape_ == Qt::SizeFDiagCursor )
                resize_ = true;
        else
                resize_ = false;
      
        // Change new canvas to a active overlay.
        active_canvas_ = canvases_[index]->GetID();
        // Note this call changes internal arrange of canvases_ so index is not anymore valid.
        SetTop(active_canvas_);
        
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
    if ( resize_ )
    {
       // Here we adjust texture to correspond to possible new state of widget.
       for ( int i = 0; i < canvases_.size(); ++i)
       {
           if ( canvases_[i]->GetID() == active_canvas_)
           {
                QSize size = canvases_[i]->GetSize();
                canvases_[i]->SetCanvasSize(size.width(), size.height());
                break;
           }
       }
       
    }

    mouseDown_ = false;
    lastPosition_ = point;
    resize_ = false;
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

Qt::CursorShape UIController::UpdateMouseCursor(int x, int y, int index)
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

        QRect top_side_box(QPoint(bottomLeft + corner_margin, top), QPoint(bottomRight - corner_margin, top + side_margin));
        QRect bottom_side_box(QPoint(bottomLeft - corner_margin, bottom - side_margin), QPoint(bottomRight - corner_margin, bottom));

        QRect left_side_box(QPoint(bottomLeft, top + corner_margin), QPoint(bottomLeft + side_margin, bottom - corner_margin));
        QRect right_side_box(QPoint(bottomRight-corner_margin, top + side_margin), QPoint(bottomRight, bottom - corner_margin));

        QPoint point(x,y);

        if ( left_bottom_corner_box.contains(point) )
        {
            QApplication::setOverrideCursor(QCursor(Qt::SizeBDiagCursor));
            mouseCursorShape_ = Qt::SizeBDiagCursor;
        }
        else if ( left_top_corner_box.contains(point) )
        {
            QApplication::setOverrideCursor(QCursor(Qt::SizeFDiagCursor));
            mouseCursorShape_ = Qt::SizeFDiagCursor;
        }
        else if ( right_top_corner_box.contains(point))
        {
            QApplication::setOverrideCursor(QCursor(Qt::SizeBDiagCursor));
            mouseCursorShape_ = Qt::SizeBDiagCursor;
        }
        else if ( right_bottom_corner_box.contains(point) )
        {
            QApplication::setOverrideCursor(QCursor(Qt::SizeFDiagCursor));
            mouseCursorShape_ = Qt::SizeFDiagCursor;
        }
        else if (top_side_box.contains(point))
        {
            QApplication::setOverrideCursor(QCursor(Qt::SizeVerCursor));
            mouseCursorShape_ = Qt::SizeVerCursor;
        }
        else if (bottom_side_box.contains(point) )
        {
            QApplication::setOverrideCursor(QCursor(Qt::SizeVerCursor));
            mouseCursorShape_ = Qt::SizeVerCursor;
        }
        else if (left_side_box.contains(point))
        {
            QApplication::setOverrideCursor(QCursor(Qt::SizeHorCursor));
            mouseCursorShape_ = Qt::SizeHorCursor; 
        }
        else if (right_side_box.contains(point))
        {
            QApplication::setOverrideCursor(QCursor(Qt::SizeHorCursor));
            mouseCursorShape_ = Qt::SizeHorCursor;
        }
        else
        {
            QApplication::setOverrideCursor(QCursor(Qt::ArrowCursor));
            mouseCursorShape_ = Qt::ArrowCursor;        
            //QApplication::restoreOverrideCursor();   
        }
    }

    return mouseCursorShape_;
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
        
     
     
   

      
       

     
      
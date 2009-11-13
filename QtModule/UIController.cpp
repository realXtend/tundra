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

namespace
{
    /// This is the minimum Z order value the Qt canvases are given in the Ogre overlay order. (smaller number: farther away. max: ~600)
    const int minimumCanvasZValue = 300;
}

namespace QtUI
{

UIController::UIController()
:responseTimeLimit_(500), 
keyDown_(false), 
multipleKeyLimit_(150), 
currentMouseAction(MouseActionNone),
mouseActionCanvas(0),
mouseHoverCanvas(0),
keyboardFocusCanvas(0),
lastKnownKeyboardFocusItem(0)
{}

UIController::~UIController()
{}

void UIController::Update()
{
    ///\todo Remove and move to OIS.
    // If a key-press repeat period has elapsed, trigger a keypress event.
    if (keyDown_ && keyTimer_.elapsed() > multipleKeyLimit_)
    {
        // Restart the timer.
        keyTimer_ = QTime();
        keyTimer_.start();
    
        for (QList<QPair<Qt::Key, QString> >::iterator iter = pressedKeys_.begin(); iter != pressedKeys_.end(); ++iter)
            InjectKeyPressed((*iter).second, (*iter).first);
    }

    // Redraw any dirty canvases.
    QList<boost::shared_ptr<UICanvas> >::iterator iter = canvases_.begin();
    for(; iter != canvases_.end(); ++iter)
        (*iter)->Render();
}

int UIController::GetCanvasIndexByID(const QString& id) const
{
    for(int i = 0; i < canvases_.size(); ++i)
        if (canvases_[i]->GetID() == id)
            return i;

    return -1;
}

/** This function rearranges the Ogre overlay Z values to reflect the current UI canvas orders.
    Invalidates all existing canvas indices. */
void UIController::Arrange()
{
    // This is the biggest Z value given to a canvas. The always-on-top canvases get the biggest Z values.
    const int maximumCanvasZValue = minimumCanvasZValue + canvases_.size();

    // Move all the always-on-top canvases to front, while retaining their relative ordering.
    // Important to keep the reordering stable.
    int nAlwaysOnTopCanvases = 0;
    for(int i = 0; i < canvases_.size(); ++i)
        if (canvases_[i]->IsAlwaysOnTop())
            canvases_.move(i, nAlwaysOnTopCanvases++);

    // Re-count new Z values for the canvas Ogre overlays. canvases[0] is frontmost, canvases[size-1] is last.
    for(int i = 0; i < canvases_.size(); ++i)
        canvases_[i]->SetZOrder(maximumCanvasZValue - i);
}

void UIController::HideCanvas(UICanvas &canvas)
{
    assert(&canvas);

    if (&canvas == mouseActionCanvas)
    {
        Deactivate(*mouseActionCanvas);
        mouseActionCanvas = 0;
    }
    if (&canvas == mouseHoverCanvas)
    {
        Deactivate(*mouseHoverCanvas);
        mouseHoverCanvas = 0;
    }
    if (&canvas == keyboardFocusCanvas)
    {
        Deactivate(*keyboardFocusCanvas);
        keyboardFocusCanvas = 0;
    }

    canvas.Hide();
}

void UIController::RemoveCanvas(const QString& id)
{
    QList<boost::shared_ptr<UICanvas> >::iterator iter = canvases_.begin();
    for(; iter != canvases_.end(); ++iter)
    {
        QString canvas_id = (*iter)->GetID();
        if (canvas_id == id)
        {
            HideCanvas(**iter);
            // This *should* cause a deletion of the canvas, as long as no
            // client module has attached a shared_ptr to it. 
            canvases_.erase(iter);
            return;
        }
    }

    QString message = QString("QtModule: There does not exist a canvas with id ") + id + "!";
    throw Core::Exception(message.toStdString().c_str()); 
}

boost::weak_ptr<UICanvas> UIController::CreateCanvas(UICanvas::DisplayMode mode)
{
    boost::shared_ptr<UICanvas> canvas(new UICanvas(mode, parentWindowSize_));
    
    QObject::connect(this,SIGNAL(RenderWindowSizeChanged(const QSize&)),canvas.get(), SLOT(SetRenderWindowSize(const QSize&)));
    QObject::connect(canvas.get(), SIGNAL(ToTop(const QString&)), this, SLOT(SetTop(const QString&)));
    QObject::connect(canvas.get(), SIGNAL(ToBack(const QString&)), this, SLOT(SetBack(const QString&)));

    // Add the new canvas to be the first on the list so that it will get the highest Z order when rearranging (it'll be topmost).
    canvases_.prepend(canvas);
    Arrange();

    return canvas;
}

void UIController::InjectMouseMove(int x, int y, int deltaX, int deltaY)
{
    QPoint point(x,y);
    int index = GetCanvasIndexAt(point);
    UICanvas *currentCanvas = (index == -1 ? 0 : canvases_[index].get());

    // If we've lost our target action canvas, stop the action.
    if (!mouseActionCanvas)
        currentMouseAction = MouseActionNone;

    const int canvasWidth = (mouseActionCanvas ? mouseActionCanvas->GetSize().width() : 0);
    const int canvasHeight = (mouseActionCanvas ? mouseActionCanvas->GetSize().height() : 0);

    switch(currentMouseAction)
    {
    case MouseActionNone:
        // If the mouse hover canvas has changed, deactivate the old canvas from any old mouse hover events, and save the new one.
        if (mouseHoverCanvas != currentCanvas)
        {
            if (mouseHoverCanvas)
                Deactivate(*mouseHoverCanvas, MouseMove);

            mouseHoverCanvas = currentCanvas;
        }

        // If currentCanvas == 0, this will default to the arrow cursor.
        UpdateMouseCursor(currentCanvas, x, y);

        if (currentCanvas)
            SendMouseMoveEvent(*currentCanvas, x, y);
        break;

    case MouseActionCanvasInternal:
        // There is a mouse drag or similar occurring in the canvas. That canvas steals the mouse focus
        // so send the mouse event to it.
        SendMouseMoveEvent(*mouseActionCanvas, x, y);
        break;

    case MouseActionCanvasMove:
        {
            QPointF pos = mouseActionCanvas->GetPosition();
            QPoint p = pos.toPoint();
            pos.setX(p.x() + deltaX);
            pos.setY(p.y() + deltaY);
            
            mouseActionCanvas->SetPosition(pos.x(), pos.y());
        }
        break;
    case MouseActionCanvasResizeTopLeft:
        mouseActionCanvas->Resize(canvasWidth - deltaX, canvasHeight - deltaY, UICanvas::BottomRight);
        break;
    case MouseActionCanvasResizeTop:
        mouseActionCanvas->Resize(canvasWidth, canvasHeight - deltaY, UICanvas::BottomRight);
        break;
    case MouseActionCanvasResizeTopRight:
        mouseActionCanvas->Resize(canvasWidth + deltaX, canvasHeight - deltaY, UICanvas::BottomLeft);
        break;
    case MouseActionCanvasResizeLeft:
        mouseActionCanvas->Resize(canvasWidth - deltaX, canvasHeight, UICanvas::BottomRight);
        break;
    case MouseActionCanvasResizeRight:
        mouseActionCanvas->Resize(canvasWidth + deltaX, canvasHeight, UICanvas::BottomLeft);
        break;
    case MouseActionCanvasResizeBottomLeft:
        mouseActionCanvas->Resize(canvasWidth - deltaX, canvasHeight + deltaY, UICanvas::TopRight);
        break;
    case MouseActionCanvasResizeBottom:
        mouseActionCanvas->Resize(canvasWidth, canvasHeight + deltaY, UICanvas::TopRight);
        break;
    case MouseActionCanvasResizeBottomRight:
        mouseActionCanvas->Resize(canvasWidth + deltaX, canvasHeight + deltaY, UICanvas::TopLeft);
        break;
    }
}

void UIController::SetTop(const QString& id) 
{
    int index = GetCanvasIndexByID(id);
    if (index == -1)
        return;

    // Bring the canvas topmost and rearrange Z orders.
    // Arrange will guarantee that always-on-top canvases retake their position on top.
    canvases_.swap(index, 0);
    Arrange(); 
}

void UIController::SetBack(const QString& id)
{
    int index = GetCanvasIndexByID(id);
    if (index == -1)
        return;

    // Move this canvas to back and rearrange Z orders.
    canvases_.swap(index, canvases_.size()-1);
    Arrange();
}


void UIController::InjectMousePress(int x, int y)
{
    QPoint point(x,y);
  
    int index = GetCanvasIndexAt(point);
    
    UICanvas *currentCanvas = (index == -1 ? 0 : canvases_[index].get());

    // Update the current mouse hover canvas to be the current one, if not so yet.
    if (mouseHoverCanvas != currentCanvas)
    {
        if (mouseHoverCanvas)
            Deactivate(*mouseHoverCanvas, MouseMove);

        mouseHoverCanvas = currentCanvas;
    }

    if (!currentCanvas)
    {
        // Clicked on top of an empty area - clear all mouse action status and return.
        doubleClickTimer_ = QTime(); 

        mouseActionCanvas = 0;
        currentMouseAction = MouseActionNone;

        // Disable the keyboard focus from all canvases altogether.
        ActivateKeyboardFocus(0, 0, 0);
        return;
    }

    assert(!currentCanvas->IsHidden());

    // See if this click triggered a double-click instead.
    if (!doubleClickTimer_.isNull() && doubleClickTimer_.elapsed() <= responseTimeLimit_)
    {
        doubleClickTimer_ = QTime();
        
        QPoint pointOnCanvas = currentCanvas->MapToCanvas(point.x(), point.y());
        QPoint clickDelta = pointOnCanvas - lastMousePressPoint_;
        if (clickDelta.manhattanLength() < 2)
        {
            InjectDoubleClick(x,y);
            return;
        }
    }
    else 
    {
        // If not, start a new double-click timer.
        doubleClickTimer_ = QTime();
        doubleClickTimer_.start();
    }

    // Inject this mouse press to Qt.
    SendMouseLeftButtonPressEvent(*currentCanvas, x, y);

    // Activate the keyboard focus on this canvas as well.
    ActivateKeyboardFocus(currentCanvas, x, y);

    // Reset the mouse cursor, for some reason it seems to get lost after sending the LMB click message.
    ///\todo Possibly set Ogre to set a null cursor to the main window so we can handle this at will.
    /// This should become redundant after that.
    UpdateMouseCursor(currentCanvas, x, y);

    // Check if this mouse press should start a mouse action on the canvas (drag or resize).
    mouseActionCanvas = currentCanvas;
    SetTop(currentCanvas->GetID());

    // We are either performing an action on the canvas, or in the canvas.
    MouseAction hotSpotAction = DetectCanvasHotSpot(*currentCanvas, x, y);
    if (hotSpotAction != MouseActionNone)
        currentMouseAction = hotSpotAction;
    else // Mouse action inside the canvas.
        currentMouseAction = MouseActionCanvasInternal;
}

void UIController::InjectMousePress(Core::Real u, Core::Real v, const boost::shared_ptr<UICanvas>& canvas)
{
   QPointF pos = canvas->GetPosition();
   QSize size = canvas->GetSize();
   int x = u * size.width() + pos.x();
   int y = v * size.height() + pos.y();
   SendMouseLeftButtonPressEvent(*canvas.get(), x, y);
   // Now be brutal and make canvas to redraw it. 
   canvas->dirty_ = true;
   canvas->RenderSceneToOgreSurface();

}

void UIController::ActivateKeyboardFocus(UICanvas *canvas, int x, int y)
{
    if (canvas != keyboardFocusCanvas)
    {
        if (lastKnownKeyboardFocusItem)
            lastKnownKeyboardFocusItem->clearFocus();
        lastKnownKeyboardFocusItem = 0;

        QWidget *focusWidget = QApplication::focusWidget();
        if (focusWidget != 0)
            focusWidget->clearFocus();
    }

    keyboardFocusCanvas = canvas;

    if (!canvas)
        return;

    QPoint pos = canvas->MapToCanvas(x, y);

    // Remember which widget got the keyboard focus.
    lastKnownKeyboardFocusItem = canvas->view_->scene()->itemAt(pos);
}

void UIController::InjectMouseRelease(int x, int y)
{
    ////todo what to do release after double click?

    currentMouseAction = MouseActionNone;

    if (mouseActionCanvas == 0)
        return;

    // Translate the mouse position from QGraphicsView coordinate frame onto
    // the QGraphicsScene coordinate frame.
    QPoint point(x,y);

    QPoint pos = mouseActionCanvas->MapToCanvas(x,y);
    QPoint currentMousePos(pos.x(), pos.y());

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

    QApplication::sendEvent(mouseActionCanvas->view_->scene(), &mouseEvent);

    // Reset the mouse cursor, for some reason it seems to get lost after sending the LMB release message.
    ///\todo Possibly set Ogre to set a null cursor to the main window so we can handle this at will.
    /// This should become redundant after that.
    UpdateMouseCursor(mouseActionCanvas, x, y);

    mouseActionCanvas = 0;
}

void UIController::InjectDoubleClick(int x, int y)
{
    QPoint point(x,y);
    int index = GetCanvasIndexAt(point);
    
    if (index != -1 && !canvases_[index]->IsHidden())
    {
        // Translate the mouse position from QGraphicsView coordinate frame onto
        // the QGraphicsScene coordinate frame.
        QPoint pos = canvases_[index]->MapToCanvas(x,y);
        QPoint currentMousePos(pos.x(), pos.y());

        currentMouseAction = MouseActionNone;
        
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
     
        QApplication::sendEvent(canvases_[index]->view_->scene(), &mouseEvent);
    }
}

void UIController::InjectKeyPressed(const QString& text, Qt::Key keyCode, const Qt::KeyboardModifiers& modifier)
{
    if (!keyboardFocusCanvas)
        return;

    ///\todo Check invariant: keyboardFocusCanvas should be at the top in Z order, and not hidden.

    QKeyEvent keyEvent(QEvent::KeyPress, keyCode, modifier, text);
    keyEvent.setAccepted(false);
    
    // Check that the canvas is ready to take the input.
    if (!keyboardFocusCanvas->IsActiveWindow())
        keyboardFocusCanvas->Activate();

     QApplication::sendEvent(keyboardFocusCanvas->view_->scene(), &keyEvent);
     keyDown_ = true;
     keyTimer_.start();

     // Add key into list (if it is unique).
     if (!pressedKeys_.contains(qMakePair(keyCode, text)))
          pressedKeys_.append(qMakePair(keyCode, text));
}

void UIController::InjectKeyReleased(const QString& text, Qt::Key keyCode, const Qt::KeyboardModifiers& modifier)
{
    if (!keyboardFocusCanvas)
        return;

    QKeySequence sequence(keyCode);  

    QKeyEvent keyEvent(QEvent::KeyRelease, keyCode, modifier, sequence.toString().toLower());
    keyEvent.setAccepted(false);
    
    QApplication::sendEvent(keyboardFocusCanvas->view_->scene(), &keyEvent);
    keyTimer_ = QTime();
  
    int size = pressedKeys_.size()-1;
    for(int i = size; i > 0; --i)
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

UIController::MouseAction UIController::DetectCanvasHotSpot(UICanvas &canvas, int x, int y)
{
    assert(&canvas);

    QRect frame = canvas.view_->frameGeometry();
    QPointF pos = canvas.GetPosition();
    
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

    // Check all the eight rectangles for the canvas resize.
    if (canvas.appearPolicy_->IsResizable())
    {
        if (left_bottom_corner_box.contains(point))  return MouseActionCanvasResizeBottomLeft;
        if (left_top_corner_box.contains(point))     return MouseActionCanvasResizeTopLeft;
        if (right_top_corner_box.contains(point))    return MouseActionCanvasResizeTopRight;
        if (right_bottom_corner_box.contains(point)) return MouseActionCanvasResizeBottomRight;
        if (top_side_box.contains(point))    return MouseActionCanvasResizeTop;
        if (bottom_side_box.contains(point)) return MouseActionCanvasResizeBottom;
        if (left_side_box.contains(point))   return MouseActionCanvasResizeLeft;
        if (right_side_box.contains(point))  return MouseActionCanvasResizeRight;
    }

    if (!canvas.IsPositionStationary())
    {
        int drag_corner_margin = 8;
        int drag_side_margin = 20;

        // Check the top title bar for the canvas move.
        QRect drag_top_side_box(QPoint(bottomLeft + drag_corner_margin, top), QPoint(bottomRight - drag_corner_margin, top + drag_side_margin));
        if (drag_top_side_box.contains(QPoint(x,y)))
            return MouseActionCanvasMove;
    }

    return MouseActionNone;
}

void UIController::UpdateMouseCursor(UICanvas *canvas, int x, int y)
{
    Qt::CursorShape shape = Qt::ArrowCursor;
    MouseAction action = (canvas == 0 ? MouseActionNone : DetectCanvasHotSpot(*canvas, x, y));
    switch(action)
    {
    case MouseActionCanvasResizeTopLeft: 
    case MouseActionCanvasResizeBottomRight:
        shape = Qt::SizeFDiagCursor;
        break;

    case MouseActionCanvasResizeTop:
    case MouseActionCanvasResizeBottom:
        shape = Qt::SizeVerCursor;
        break;

    case MouseActionCanvasResizeTopRight:
    case MouseActionCanvasResizeBottomLeft:
        shape = Qt::SizeBDiagCursor;
        break;

    case MouseActionCanvasResizeLeft:
    case MouseActionCanvasResizeRight:
        shape = Qt::SizeHorCursor;
        break;

    default:
        if (canvas)
        {
            QPoint pos = canvas->MapToCanvas(x, y);

            QGraphicsItem *item = canvas->view_->scene()->itemAt(pos);
            QGraphicsWidget *widget = (item ? item->topLevelWidget() : 0);
            QWidget *focusWidget = QApplication::focusWidget();

            if (item && item->hasCursor())
                shape = item->cursor().shape();
            else if (widget && widget->hasCursor())
                shape = widget->cursor().shape();
            else if (focusWidget)
                shape = focusWidget->cursor().shape();
            else    
                shape = Qt::ArrowCursor;
        }
        break;
    }

    QApplication::setOverrideCursor(QCursor(shape));
}

void UIController::Deactivate(UICanvas &canvas, DeactivationType type)
{
    assert(&canvas);

    // We assume that the upper-left corner of a canvas is the safe location to post a mouse move event 
    // so that it clears any previous hover visual animation.
    QPoint pos = canvas.GetPosition().toPoint();

    switch(type)
    {
    case MouseMove:
        SendMouseMoveEvent(canvas, pos.x(), pos.y());
        break;
    case MousePress:
        ///todo send mouse press
        break;
    case MouseRelease:
        ///todo send mouse release
        break;
    case All:
        ///todo send mouse press and release
        SendMouseMoveEvent(canvas, pos.x(), pos.y());
        break;
    }
}

UICanvas *UIController::GetCanvasAt(int x, int y)
{
    const QPoint point(x, y);
    for(QList<boost::shared_ptr<UICanvas> >::iterator iter = canvases_.begin(); iter != canvases_.end(); ++iter)
        if ((*iter)->GetDisplayMode() == UICanvas::Internal && !(*iter)->IsHidden() && Contains(**iter, point))
            return iter->get();

    return 0;
}

boost::weak_ptr<QtUI::UICanvas> UIController::GetCanvasByID(const QString& id)
{
    for(QList<boost::shared_ptr<UICanvas> >::iterator iter = canvases_.begin(); 
        iter != canvases_.end(); ++iter)
        if ((*iter)->GetID() == id)
            return *iter;

    return boost::weak_ptr<UICanvas>();
}

int UIController::GetCanvasIndexAt(const QPoint& point) 
{
    int index = 0;

    QList<boost::shared_ptr<UICanvas> >::iterator iter = canvases_.begin();
    for(; iter != canvases_.end(); ++iter, ++index)
    {
        // If mode is external we let the Qt own window manager to things. 
        if ((*iter)->GetDisplayMode() == UICanvas::External)
            continue;
        if ((*iter)->IsHidden())
            continue;
        
        if (Contains(**iter, point))
            return index;
    }

    // Did not find any canvases so let's return -1.
    return -1;
}

bool UIController::Contains(const UICanvas &canvas, const QPoint& point) const
{
    assert(&canvas);

    const QPoint pos = canvas.GetPosition().toPoint();

    return pos.x() <= point.x() && pos.y() <= point.y() && 
           pos.x() + canvas.view_->width() > point.x() && 
           pos.y() + canvas.view_->height() > point.y();
}

void UIController::SendMouseMoveEvent(UICanvas &canvas, int x, int y)
{
    assert(&canvas);

    // Map the mouse coordinate to the scene of the canvas.
    QPoint pos = canvas.MapToCanvas(x,y);

    QGraphicsSceneMouseEvent mouseEvent(QEvent::GraphicsSceneMouseMove);

    bool mouseLeftButtonDown = (currentMouseAction != MouseActionNone);
    if (mouseLeftButtonDown)
    {
        mouseEvent.setButtonDownScenePos(Qt::LeftButton, lastMousePressPoint_);
        mouseEvent.setButtonDownScreenPos(Qt::LeftButton, lastMousePressPoint_);
    }
    else
    {
        mouseEvent.setButtonDownScenePos(Qt::NoButton, pos);
        mouseEvent.setButtonDownScreenPos(Qt::NoButton, pos);
    }
    
    mouseEvent.setScenePos(pos);
    mouseEvent.setScreenPos(pos);
    mouseEvent.setLastScenePos(pos);
    mouseEvent.setLastScreenPos(pos);
    mouseEvent.setButtons(mouseLeftButtonDown ? Qt::LeftButton : Qt::NoButton);
    mouseEvent.setButton(mouseLeftButtonDown ? Qt::LeftButton : Qt::NoButton);
    mouseEvent.setModifiers(0);
    mouseEvent.setAccepted(false);

    QApplication::sendEvent(canvas.view_->scene(), &mouseEvent);   
}

void UIController::SendMouseLeftButtonPressEvent(UICanvas &canvas, int x, int y)
{
    assert(&canvas);

    QPoint pos = canvas.MapToCanvas(x, y);

    lastMousePressPoint_ = pos;

    QGraphicsSceneMouseEvent mouseEvent(QEvent::GraphicsSceneMousePress);
    
    mouseEvent.setButtonDownScenePos(Qt::LeftButton, pos);
    mouseEvent.setButtonDownScreenPos(Qt::LeftButton, pos);
    mouseEvent.setScenePos(pos);
    mouseEvent.setScreenPos(pos);
    mouseEvent.setLastScenePos(pos);
    mouseEvent.setLastScreenPos(pos);
    mouseEvent.setButtons(Qt::LeftButton);
    mouseEvent.setButton(Qt::LeftButton);

    mouseEvent.setModifiers(0);
    mouseEvent.setAccepted(false);

    // Make this canvas the current active canvas for the OS so that it is properly prepared to take our input.
    if (!canvas.IsActiveWindow())
        canvas.Activate();

    QApplication::sendEvent(canvas.view_->scene(), &mouseEvent);
}

}

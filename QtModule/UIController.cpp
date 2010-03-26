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
currentMouseAction(MouseActionNone),
mouseActionCanvas(0),
mouseHoverCanvas(0),
keyboardFocusCanvas(0),
lastKnownKeyboardFocusItem(0),
currentModifier_(Qt::NoModifier),
button_(Qt::LeftButton)
{}

UIController::~UIController()
{}

void UIController::Update()
{
    
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
        lastKnownKeyboardFocusItem = 0;
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
    throw Exception(message.toStdString().c_str()); 
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

void UIController::InjectMouseMove(int x, int y, int deltaX, int deltaY, UICanvas* canvas)
{
    QPoint point(x,y);
    
    int index = GetCanvasIndexAt(point);
    UICanvas *currentCanvas = (index == -1 ? 0 : canvases_[index].get());
    

    if ( currentCanvas == 0 && canvas != 0)
        currentCanvas = canvas;

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
            if ( button_ != Qt::RightButton)
            {
                QPointF pos = mouseActionCanvas->GetPosition();
                QPoint p = pos.toPoint();
                pos.setX(p.x() + deltaX);
                pos.setY(p.y() + deltaY);
                
                mouseActionCanvas->SetPosition(pos.x(), pos.y());
            }
        }
        break;
    case MouseActionCanvasResizeTopLeft:
        if ( button_ != Qt::RightButton)
            mouseActionCanvas->Resize(canvasWidth - deltaX, canvasHeight - deltaY, UICanvas::BottomRight);
        break;
    case MouseActionCanvasResizeTop:
        if ( button_ != Qt::RightButton)
            mouseActionCanvas->Resize(canvasWidth, canvasHeight - deltaY, UICanvas::BottomRight);
        break;
    case MouseActionCanvasResizeTopRight:
        if ( button_ != Qt::RightButton)
            mouseActionCanvas->Resize(canvasWidth + deltaX, canvasHeight - deltaY, UICanvas::BottomLeft);
        break;
    case MouseActionCanvasResizeLeft:
        if ( button_ != Qt::RightButton)
            mouseActionCanvas->Resize(canvasWidth - deltaX, canvasHeight, UICanvas::BottomRight);
        break;
    case MouseActionCanvasResizeRight:
        if ( button_ != Qt::RightButton)
            mouseActionCanvas->Resize(canvasWidth + deltaX, canvasHeight, UICanvas::BottomLeft);
        break;
    case MouseActionCanvasResizeBottomLeft:
        if ( button_ != Qt::RightButton)
            mouseActionCanvas->Resize(canvasWidth - deltaX, canvasHeight + deltaY, UICanvas::TopRight);
        break;
    case MouseActionCanvasResizeBottom:
        if ( button_ != Qt::RightButton)
            mouseActionCanvas->Resize(canvasWidth, canvasHeight + deltaY, UICanvas::TopRight);
        break;
    case MouseActionCanvasResizeBottomRight:
        if ( button_ != Qt::RightButton)
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


void UIController::InjectMousePress(int x, int y, UICanvas* canvas)
{
    QPoint point(x,y);
  
    UICanvas* currentCanvas = canvas;
    
    /*
    int index = GetCanvasIndexAt(point);
    UICanvas *currentCanvas = (index == -1 ? 0 : canvases_[index].get());
    if ( currentCanvas == 0 && canvas != 0)
        currentCanvas = canvas;
    */

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


    // See if this click triggered a double-click instead.
    if (!doubleClickTimer_.isNull() && doubleClickTimer_.elapsed() <= responseTimeLimit_)
    {
        doubleClickTimer_ = QTime();
        
        QPoint pointOnCanvas = currentCanvas->MapToCanvas(point.x(), point.y());
        QPoint clickDelta = pointOnCanvas - lastMousePressPoint_;
        if (clickDelta.manhattanLength() < 2)
        {
            InjectDoubleClick(x,y, currentCanvas);
            return;
        }
    }
    else 
    {
        // If not, start a new double-click timer.
        doubleClickTimer_ = QTime();
        doubleClickTimer_.start();
    }

    // Activate the keyboard focus on this canvas as well.
    // Important! This event must be sent BEFORE sending the keypress, because otherwise ActivateKeyboardFocus
    // will trigger the sending of focusOutEvent on the widget that was just pressed, which will cause
    // the press to be immediately drowned!
    ActivateKeyboardFocus(currentCanvas, x, y);

    // Inject this mouse press to Qt.
    SendMouseButtonPressEvent(*currentCanvas, x, y);

    // Reset the mouse cursor, for some reason it seems to get lost after sending the mouse click message.
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

void UIController::InjectMouseRelease(int x, int y, UICanvas* canvas)
{
    ////todo what to do release after double click?

    currentMouseAction = MouseActionNone;

    if ( mouseActionCanvas == 0)
        return;

    /*
    if (mouseActionCanvas == 0)
        if ( canvas != 0)
            mouseActionCanvas = canvas;
        else
            return;
    */

    // Translate the mouse position from QGraphicsView coordinate frame onto
    // the QGraphicsScene coordinate frame.
   
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
    mouseEvent.setButton(button_);
    mouseEvent.setModifiers(0);
    mouseEvent.setAccepted(false);

    QApplication::sendEvent(mouseActionCanvas->view_->scene(), &mouseEvent);

    // Reset the mouse cursor, for some reason it seems to get lost after sending the mouse release message.
    ///\todo Possibly set Ogre to set a null cursor to the main window so we can handle this at will.
    /// This should become redundant after that.
    UpdateMouseCursor(mouseActionCanvas, x, y);

    mouseActionCanvas = 0;
}

void UIController::InjectDoubleClick(int x, int y, UICanvas* canvas)
{
   
    if ( canvas == 0)
        return;
   
    QPoint pos = canvas->MapToCanvas(x,y);    
    QPoint currentMousePos(pos.x(), pos.y());

    currentMouseAction = MouseActionNone;
    
    // In case of double click we button generated click -> so it is set down. 
    QGraphicsSceneMouseEvent mouseEvent(QEvent::GraphicsSceneMouseDoubleClick);

    mouseEvent.setButtonDownScenePos(button_, lastMousePressPoint_);
    mouseEvent.setButtonDownScreenPos(button_, lastMousePressPoint_);
    mouseEvent.setScenePos(currentMousePos);
    mouseEvent.setScreenPos(currentMousePos);
    mouseEvent.setLastScenePos(currentMousePos);
    mouseEvent.setLastScreenPos(currentMousePos);
    mouseEvent.setButtons(button_);
    mouseEvent.setButton(button_);
    mouseEvent.setModifiers(currentModifier_);
    mouseEvent.setAccepted(false);

    QApplication::sendEvent(canvas->view_->scene(), &mouseEvent);
    
}

void UIController::InjectKeyPressed(QString& text, Qt::Key keyCode, const Qt::KeyboardModifiers& modifier)
{
    if (!keyboardFocusCanvas)
        return;

    if ( text == QString(QChar(0)) && keyCode != Qt::Key_unknown )
    {

        ///todo make somekind check function. 
        if ( keyCode != Qt::Key_Shift &&
             keyCode != Qt::Key_Control &&
             keyCode != Qt::Key_CapsLock &&
             keyCode != Qt::Key_Alt &&
             keyCode != Qt::Key_AltGr)
        {
            // Match text using Qt:s keycode. 
            
            QKeySequence sequence(keyCode);
            text = sequence.toString().toLower();

            switch  (modifier)
            {
                case Qt::ShiftModifier:
                {
                    text = text.toUpper();
                    break;
                }
                case Qt::ControlModifier:
                {
                    text = text.toUpper();
                    break;
                }
                default:
                    break;
            }
        }
    }


    ///\todo Check invariant: keyboardFocusCanvas should be at the top in Z order, and not hidden.

    QKeyEvent keyEvent(QEvent::KeyPress, keyCode, modifier, text);
    keyEvent.setAccepted(false);
    
    // Check that the canvas is ready to take the input.
    if (!keyboardFocusCanvas->IsActiveWindow())
        keyboardFocusCanvas->Activate();

     QApplication::sendEvent(keyboardFocusCanvas->view_->scene(), &keyEvent);
    
}

void UIController::InjectKeyReleased(const QString& text, Qt::Key keyCode, const Qt::KeyboardModifiers& modifier)
{
    if (!keyboardFocusCanvas)
        return;

    QKeySequence sequence(keyCode);  

    QKeyEvent keyEvent(QEvent::KeyRelease, keyCode, modifier, sequence.toString().toLower());
    keyEvent.setAccepted(false);
    
    QApplication::sendEvent(keyboardFocusCanvas->view_->scene(), &keyEvent);
    

 }

void UIController::InjectMouseScroll(int x, int y, int delta, UICanvas* canvas)
{
    if ( canvas == 0)
        return;

    QGraphicsSceneWheelEvent mouseEvent(QEvent::GraphicsSceneWheel);

    // Map the mouse coordinate to the scene of the canvas.
    QPoint pos = canvas->MapToCanvas(x,y);

    mouseEvent.setDelta(delta);
    mouseEvent.setOrientation(Qt::Vertical);

    bool mouseButtonDown = (currentMouseAction != MouseActionNone);
    
    mouseEvent.setScenePos(pos);
    mouseEvent.setScreenPos(pos);
    mouseEvent.setButtons(mouseButtonDown ? button_ : Qt::NoButton);


    mouseEvent.setModifiers(currentModifier_);
    mouseEvent.setAccepted(false);

    QApplication::sendEvent(canvas->view_->scene(), &mouseEvent);
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

void UIController::Redraw(const boost::shared_ptr<UICanvas>& canvas)
{
    canvas->dirty_ = true;
    canvas->RenderSceneToOgreSurface();
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

    bool mouseButtonDown = (currentMouseAction != MouseActionNone);
    
    if (mouseButtonDown)
    {
        mouseEvent.setButtonDownScenePos(button_, lastMousePressPoint_);
        mouseEvent.setButtonDownScreenPos(button_, lastMousePressPoint_);
       
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
    mouseEvent.setButtons(mouseButtonDown ? button_ : Qt::NoButton);
    mouseEvent.setButton(mouseButtonDown ? button_ : Qt::NoButton);

    mouseEvent.setModifiers(currentModifier_);
    mouseEvent.setAccepted(false);

    QApplication::sendEvent(canvas.view_->scene(), &mouseEvent);   
}

void UIController::SendMouseButtonPressEvent(UICanvas &canvas, int x, int y)
{
    assert(&canvas);

    QPoint pos = canvas.MapToCanvas(x, y);

    lastMousePressPoint_ = pos;

    QGraphicsSceneMouseEvent mouseEvent(QEvent::GraphicsSceneMousePress);
    /*
    mouseEvent.setButtonDownScenePos(Qt::LeftButton, pos);
    mouseEvent.setButtonDownScreenPos(Qt::LeftButton, pos);
    */
    mouseEvent.setButtonDownScenePos(button_,pos);
    mouseEvent.setButtonDownScreenPos(button_,pos);

    mouseEvent.setScenePos(pos);
    mouseEvent.setScreenPos(pos);
    mouseEvent.setLastScenePos(pos);
    mouseEvent.setLastScreenPos(pos);
    mouseEvent.setButton(button_);
    mouseEvent.setButtons(button_);
    /*
    mouseEvent.setButtons(Qt::LeftButton);
    mouseEvent.setButton(Qt::LeftButton);
    */

    mouseEvent.setModifiers(currentModifier_);
    mouseEvent.setAccepted(false);

    // Make this canvas the current active canvas for the OS so that it is properly prepared to take our input.
    if (!canvas.IsActiveWindow())
        canvas.Activate();

    QApplication::sendEvent(canvas.view_->scene(), &mouseEvent);
}

}

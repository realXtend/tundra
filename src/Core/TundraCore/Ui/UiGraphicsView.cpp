// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "UiGraphicsView.h"

#include "CoreException.h"
#include "Framework.h"

#include <QRect>
#include <QList>
#include <QEvent>
#include <QResizeEvent>
#include <QGraphicsItem>
#include <QMainWindow>
#include <QMenuBar>

#include <utility>

#include "MemoryLeakCheck.h"

using namespace std;

UiGraphicsView::UiGraphicsView(Framework* fw, QWidget *parent)
:QGraphicsView(parent), framework(fw), backBuffer(0)
{
    setAutoFillBackground(false);
    setAttribute(Qt::WA_NoSystemBackground, true);
    setAttribute(Qt::WA_PaintOnScreen, true);

    setFocusPolicy(Qt::StrongFocus);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setLineWidth(0);

    setUpdatesEnabled(false);
    setAttribute(Qt::WA_DontShowOnScreen, true);
}

UiGraphicsView::~UiGraphicsView()
{
    delete backBuffer;
}

QImage *UiGraphicsView::BackBuffer() const
{
    return backBuffer;
}

void UiGraphicsView::MarkViewUndirty()
{
    dirtyRectangle = QRectF(-1, -1, -1, -1);
}

bool UiGraphicsView::IsViewDirty() const
{
    return dirtyRectangle.right() >= 0;
}

QRectF UiGraphicsView::DirtyRectangle() const
{
    return dirtyRectangle;
}

void UiGraphicsView::drawBackground(QPainter *painter, const QRectF &rect)
{
    // Default backgroudBrush for QGraphicsScene and QGraphicsView is NoBrush,
    // so this will essentially just paint the animated ether login screen background.
    // Inworld and other scene should not do anything.
    return QGraphicsView::drawBackground(painter, rect);
}

bool UiGraphicsView::event(QEvent *event)
{
    if (event->type() == QEvent::UpdateRequest || event->type() == QEvent::Paint || event->type() == QEvent::Wheel)
        return true;
/*
    if (event->type() == QEvent::MetaCall)
    {
        QMetaCallEvent *mce = static_cast<QMetaCallEvent*>(event);
        if (mce->signalId() == 31)
            return true;
    }
*/
    return QGraphicsView::event(event);
}

void UiGraphicsView::Resize(int newWidth, int newHeight)
{
    /// @note As long as this UiGraphicsView is the central widget of our mainwindow
    /// we cannot set the size with plain mainwindow size. We will get auto resized.
    /// Also this size does not take into account possible menubar in the mainwindow and
    /// so this input size is not correct. We will do calculations in resizeEvent(), see there for more info.

    if (framework->HasCommandLineParameter("--nocentralwidget"))
    {
        newWidth = max(1, newWidth);
        newHeight = max(1, newHeight);

        setGeometry(0, 0, newWidth, newHeight);
        viewport()->setGeometry(0, 0, newWidth, newHeight);
        scene()->setSceneRect(viewport()->rect());
        dirtyRectangle = QRectF(0, 0, newWidth, newHeight);

        delete backBuffer;
        backBuffer = new QImage(newWidth, newHeight, QImage::Format_ARGB32);

        emit WindowResized(newWidth, newHeight);

        // qDebug() << "Internal resize: " << size() << " viewport" << viewport()->size() << " scene size: " << sceneRect().size();
    }
}

void UiGraphicsView::resizeEvent(QResizeEvent *e)
{
    QGraphicsView::resizeEvent(e);

    /// @bug Qt has a nasty bug with QGraphicsView/QGraphicsScene that leaves 1 pixel padding around
    /// the scene and the view. For us this is problematic as we have no Qt rendering in the parent that would
    /// fill out the gap with something eye pleasing. For us it leaves rendering artifacts in the 'backbuffer'.
    /// Also if we try to keep the view/scene sizes in sync we end up with a bigger scene than the view has to offer space for.
    /// This introduces then a 'mouse scroll' bug that scroll the scene when mouse scroll is done over certain QGraphicsObjects
    /// that don't accept mouse scroll and it get propagated to the view that in turn moves the viewport to show the out of bounds scene.
    /// Here we try to go around this issue, earlier we had a similar hack and this is the replacing hack when the UiGraphicsView is the 
    /// central widget of the mainwindow. Inlike mainwindow->height() we need to take into considerations the menubar that is outside
    /// or the central widgets rendering space, we cannot use mainwindow->height() directly as we could before.
    /// TL;DR This is a hack to get rid of rendering artifacts and it will remain as such untill Qt fixes this or we find a proper solution.

    QMainWindow *parentMainWin = dynamic_cast<QMainWindow*>(parentWidget());
    int parentWidth = max(1, parentMainWin ? parentMainWin->geometry().width() : 0);
    int parentHeight = max(1, parentWidget() ? parentMainWin->geometry().height() : 0);
    parentHeight -= (parentMainWin && parentMainWin->menuBar()) ? parentMainWin->menuBar()->geometry().height() : 0;

    QRect newGeom(0, 0, max(parentWidth, e->size().width()), max(parentHeight, e->size().height()));

    // Always keep the scene same size as the view is.
    if (viewport())
        viewport()->setGeometry(newGeom);
    if (scene())
        scene()->setSceneRect(viewport()->geometry());

    // Update our rendering backbuffer.
    delete backBuffer;
    backBuffer = new QImage(newGeom.width(), newGeom.height(), QImage::Format_ARGB32);

    emit WindowResized(newGeom.width(), newGeom.height());

/*
    if (!viewport() || !scene())
        return; ///\todo LogWarning.

    int newWidth = max(1, width());
    int newHeight = max(1, height());
#if 0 ///\todo enable.
    // Resize render window and UI texture to match this
    if (view_)
    {
        view_->ResizeWindow(width(), height());
        view_->ResizeOverlay(viewport()->width(), viewport()->height());
        main_window_->resize(width(), height());
//            if (viewport()->width() != width() || viewport()->height() != height())
//                viewport()->setGeometry(0, 0, width(), height());
//            viewport()->resize(width(), height());

//        if (horizontalScrollBarPolicy() != Qt::ScrollBarAlwaysOff)
//            setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
//        horizontalScrollBar()->setValue(0);
//        horizontalScrollBar()->setRange(0, 0);

//       if (verticalScrollBarPolicy() != Qt::ScrollBarAlwaysOff)
//            setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
//        verticalScrollBar()->setValue(0);
//        verticalScrollBar()->setRange(0, 0);

    }
#endif    
    viewport()->setGeometry(0, 0, width(), height());
    scene()->setSceneRect(viewport()->rect());          
    dirtyRectangle = QRectF(0, 0, width(), height());

    delete backBuffer;
    backBuffer = new QImage(newWidth, newHeight, QImage::Format_ARGB32);

    emit WindowResized(newWidth, newHeight);

//        viewport()->setGeometry(0, 0, backBuffer->width()-1, backBuffer->height()-1);
*/
}

void UiGraphicsView::HandleSceneChanged(const QList<QRectF> &rectangles)
{
    using namespace std;

#ifndef DIRECTX_ENABLED
    // We received an unknown-sized scene change message. Mark everything dirty! (I've no idea what Qt
    // means when it sends a message saying 'nothing changed').
    if (rectangles.size() == 0)
        dirtyRectangle = QRectF(0, 0, width(), height());
#endif

    if (!IsViewDirty() && rectangles.size() > 0)
        dirtyRectangle = rectangles[0];

    // Include an extra guardband pixel to avoid graphical artifacts from occurring when redrawing.
    const int guardbandWidth = 5;

    for(int i = 0; i < rectangles.size(); ++i)
    {
        dirtyRectangle.setLeft(min(dirtyRectangle.left(), rectangles[i].left()-guardbandWidth));
        dirtyRectangle.setTop(min(dirtyRectangle.top(), rectangles[i].top()-guardbandWidth));
        dirtyRectangle.setRight(max(dirtyRectangle.right(), rectangles[i].right()+guardbandWidth));
        dirtyRectangle.setBottom(max(dirtyRectangle.bottom(), rectangles[i].bottom()+guardbandWidth));
    }
    dirtyRectangle.setLeft(max<int>(dirtyRectangle.left(), 0));
    dirtyRectangle.setTop(max<int>(dirtyRectangle.top(), 0));
    dirtyRectangle.setRight(min<int>(dirtyRectangle.right(), width()));
    dirtyRectangle.setBottom(min<int>(dirtyRectangle.bottom(), height()));
}

QGraphicsItem *UiGraphicsView::VisibleItemAtCoords(int x, int y) const
{
    // Silently just ignore any invalid coordinates we get. (and we do get them, it seems!)
    if (x < 0 || y < 0 || x >= width() || y >= height())
        return 0;

    QGraphicsItem *item = 0;
    ///\bug Not sure if this function returns the items in the proper depth order! We might not get the topmost window
    /// when this loop finishes.
    QList<QGraphicsItem *> itemsUnderCoord = items(x, y);
    for(int i = 0; i < itemsUnderCoord.size(); ++i)
        if (itemsUnderCoord[i]->isVisible())
        {
            item = itemsUnderCoord[i];
            break;
        }    

    if (!item)
        return 0;

    if (!backBuffer)
        throw Exception("UiGraphicsView::backBuffer not initialized properly!");

    // Do alpha keying: If we have clicked on a transparent part of a widget, act as if we didn't click on a widget at all.
    // This allows clicks to go through to the 3D scene from transparent parts of a widget.
    if (x < backBuffer->width() && y < backBuffer->height() && (backBuffer->pixel(x, y) & 0xFF000000) == 0x00000000)
        item = 0;

    return item;
}

void UiGraphicsView::dropEvent(QDropEvent *e)
{
    // There was no widget on top of the are where the event occurred. Pass the drop event on as a global "3D scene drag-n-drop event".
    // Applications can register to this signal to perform custom 3D scene drag-n-drop handling.
    QGraphicsItem *underMouse = GetVisibleItemAtCoords(e->pos().x(), e->pos().y());
    emit DropEvent(e, underMouse);
}

void UiGraphicsView::dragEnterEvent(QDragEnterEvent *e)
{
    // There was no widget on top of the are where the event occurred. Pass the drop event on as a global "3D scene drag-n-drop event".
    // Applications can register to this signal to perform custom 3D scene drag-n-drop handling.
    QGraphicsItem *underMouse = VisibleItemAtCoords(e->pos().x(), e->pos().y());
    emit DragEnterEvent(e, underMouse);
}

void UiGraphicsView::dragLeaveEvent(QDragLeaveEvent *e)
{
    emit DragLeaveEvent(e);
}

void UiGraphicsView::dragMoveEvent(QDragMoveEvent *e)
{
    // There was no widget on top of the are where the event occurred. Pass the drop event on as a global "3D scene drag-n-drop event".
    // Applications can register to this signal to perform custom 3D scene drag-n-drop handling.
    QGraphicsItem *underMouse = VisibleItemAtCoords(e->pos().x(), e->pos().y());
    emit DragMoveEvent(e, underMouse);
}

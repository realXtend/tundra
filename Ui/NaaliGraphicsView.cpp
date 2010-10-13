// For conditions of distribution and use, see copyright notice in license.txt

#include "DebugOperatorNew.h"

#include "NaaliGraphicsView.h"

#include <QRect>
#include <QList>
#include <QEvent>
#include <QResizeEvent>
#include <utility>

#include "MemoryLeakCheck.h"

using namespace std;

NaaliGraphicsView::NaaliGraphicsView(QWidget *parent)
:QGraphicsView(parent), backBuffer(0)
{
    setAutoFillBackground(false);
    setUpdatesEnabled(false);
    setAttribute(Qt::WA_NoSystemBackground, true);
    setAttribute(Qt::WA_PaintOnScreen, true);

    setFocusPolicy(Qt::StrongFocus);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setLineWidth(0);

    setAttribute(Qt::WA_DontShowOnScreen, true);
}

QImage *NaaliGraphicsView::BackBuffer()
{ 
    return backBuffer;
}

void NaaliGraphicsView::MarkViewUndirty()
{
    dirtyRectangle = QRectF(-1, -1, -1, -1);
}

bool NaaliGraphicsView::IsViewDirty() const
{
    return dirtyRectangle.right() >= 0;
}

QRectF NaaliGraphicsView::DirtyRectangle() const
{ 
    return dirtyRectangle;
}

void NaaliGraphicsView::drawBackground(QPainter *, const QRectF &)
{
    return;
}

bool NaaliGraphicsView::event(QEvent *event)
{
    if (event->type() == QEvent::UpdateRequest || event->type() == QEvent::Paint || event->type() == QEvent::Wheel)// || event->type() == QEvent::Resize)
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

void NaaliGraphicsView::Resize(int newWidth, int newHeight)
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
}

void NaaliGraphicsView::resizeEvent(QResizeEvent *e)
{
    QGraphicsView::resizeEvent(e);
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

void NaaliGraphicsView::HandleSceneChanged(const QList<QRectF> &rectangles)
{
    using namespace std;

    // We received an unknown-sized scene change message. Mark everything dirty! (I've no idea what Qt
    // means when it sends a message saying 'nothing changed').
    if (rectangles.size() == 0)
        dirtyRectangle = QRectF(0, 0, width(), height());

    if (!IsViewDirty())
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

// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "UiFwd.h"
#include "TundraCoreApi.h"

#include <QGraphicsView>

class QDropEvent;
class QDragEnterEvent;
class QDragMoveEvent;
class QMouseEvent;
class QWheelEvent;

/// The main view consists of a single QGraphicsView that spans the whole viewport.
/** UiGraphicsView implements pixel-perfect alpha-tested mouse hotspots for Qt widgets to
    determine whether clicks should go to the scene or to Qt widgets. */
class TUNDRACORE_API UiGraphicsView : public QGraphicsView
{
    Q_OBJECT

public:
    explicit UiGraphicsView(Framework* fw, QWidget *parent);

    ~UiGraphicsView();

    /// Returns the currently shown UI content as an image.
    QImage *BackBuffer() const;

    /// Returns true if there are rectangles pending repaint in the view.
    bool IsViewDirty() const;

    /// Marks the whole UI screen dirty, pending a full repaint.
    void MarkViewUndirty();

    /// Returns the rectangle that represents the dirty area of the screen, pending a Qt repaint.
    QRectF DirtyRectangle() const;

public slots:
    /// Returns the topmost visible QGraphicsItem in the given application main window coordinates.
    QGraphicsItem *VisibleItemAtCoords(int x, int y) const;
    QGraphicsItem *GetVisibleItemAtCoords(int x, int y) const { return VisibleItemAtCoords(x,y); } ///< @deprecated Use VisibleItemAtCoords. @todo Add warning print

    /// Sets a new size for this widget. Will emit the WindowResized signal.
    void Resize(int newWidth, int newHeight);

signals:
    /// Emitted when this widget has been resized to a new size.
    void WindowResized(int newWidth, int newHeight);

    /// Emitted when DragEnterEvent is received for the main window.
    /** @param e Drag enter event from Qt as is.
        @param widgetUnderMouse Graphics item that is under the drop event, null if no widget. */
    void DragEnterEvent(QDragEnterEvent *e, QGraphicsItem *widgetUnderMouse);

    /// Emitted when DragLeaveEvent is received for the main window.
    /** @param e Drag leave event from Qt as is. */
    void DragLeaveEvent(QDragLeaveEvent *e);

    /// Emitted when DragMoveEvent is received for the main window.
    /** @param e Drage move event from Qt as is.
        @param widgetUnderMouse Graphics item that is under the drop event, null if no widget. */
    void DragMoveEvent(QDragMoveEvent *e, QGraphicsItem *widgetUnderMouse);

    /// Emitted when DropEvent is received for the main window.
    /** @param e Drop event from Qt as is.
        @param widgetUnderMouse Graphics item that is under the drop event, null if no widget. */
    void DropEvent(QDropEvent *e, QGraphicsItem *widgetUnderMouse);

private:
    Framework* framework;
    QImage *backBuffer;
    QRectF dirtyRectangle;

    /// This virtual function is overridden from the QGraphicsView original to disable any background drawing functionality.
    /// The main QGraphicsView background displays the 3D scene rendered using Ogre.
    void drawBackground(QPainter *, const QRectF &);

    /// Overridden to disable QEvent::UpdateRequest, QEvent::Paint and QEvent::Wheel events from being processed in the base class,
    /// which cause flickering on the screen (Qt internals have some hard-coded full-screen rectangle repaints on these signals).
    bool event(QEvent *event);

    void resizeEvent(QResizeEvent *e);

    // We override the Qt widget drag-n-drop events to be able to expose them as Qt signals (DragEnterEvent, DragMoveEvent and DropEvent)
    // to all client applications. The individual modules can listen to those signals to be able to perform drag-n-drop
    // handling of custom mime types.

    void dragEnterEvent(QDragEnterEvent *e);
    void dragLeaveEvent(QDragLeaveEvent *e);
    void dragMoveEvent(QDragMoveEvent *e);
    void dropEvent(QDropEvent *e);

private slots:
    void HandleSceneChanged(const QList<QRectF> &rectangles);
};

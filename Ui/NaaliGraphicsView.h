// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Core_NaaliGraphicsView_h
#define incl_Core_NaaliGraphicsView_h

#include <QGraphicsView>

#include "NaaliUiFwd.h"
#include "UiApi.h"

/// The main view of Naali consists of a single QGraphicsView that spans the whole viewport. NaaliGraphicsView
/// implements pixel-perfect alpha-tested mouse hotspots for Qt widgets to determine whether clicks should
/// go to the scene or to Qt widgets.
class UI_API NaaliGraphicsView : public QGraphicsView
{
    Q_OBJECT;

public:
    explicit NaaliGraphicsView(QWidget *parent);

    ~NaaliGraphicsView();

    /// Returns the currently shown UI content as an image.
    QImage *BackBuffer();

    /// Returns true if there are rectangles pending repaint in the view.
    bool IsViewDirty() const;

    /// Marks the whole UI screen dirty, pending a full repaing.
    void MarkViewUndirty();

    /// Returns the rectangle that represents the dirty area of the screen, pending a Qt repaint.
    QRectF DirtyRectangle() const;

signals:
    /// Emitted when this widget has been resized to a new size.
    void WindowResized(int newWidth, int newHeight);

    /// Emitted when DragEnterEvent is received for the main window.
    /** @param e Event.
    */
    void DragEnterEvent(QDragEnterEvent *e);

    /// Emitted when DragMoveEvent is received for the main window.
    /** @param e Event.
    */
    void DragMoveEvent(QDragMoveEvent *e);

    /// Emitted when DropEvent is received for the main window.
    /** @param e Event.
    */
    void DropEvent(QDropEvent *e);

public slots:
    /// Sets a new size for this widget. Will emit the WindowResized signal.
    void Resize(int newWidth, int newHeight);

private:
    QImage *backBuffer;
    QRectF dirtyRectangle;

    /// This virtual function is overridden from the QGraphicsView original to disable any background drawing functionality.
    /// The NaaliGraphicsView background displays the 3D scene rendered using Ogre.
    void drawBackground(QPainter *, const QRectF &);
    /// Overridden to disable QEvent::UpdateRequest, QEvent::Paint and QEvent::Wheel events from being processed in the base class,
    /// which cause flickering on the screen (Qt internals have some hard-coded full-screen rectangle repaints on these signals).
    bool event(QEvent *event);
    /// 
    void resizeEvent(QResizeEvent *e);
    void dragEnterEvent(QDragEnterEvent *e);
    void dragMoveEvent(QDragMoveEvent *e);
    void dropEvent(QDropEvent *e);

private slots:
    void HandleSceneChanged(const QList<QRectF> &rectangles);
};

#endif

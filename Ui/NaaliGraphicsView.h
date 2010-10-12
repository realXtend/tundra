// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Core_NaaliGraphicsView_h
#define incl_Core_NaaliGraphicsView_h

#include <QGraphicsView>

#include "NaaliUiFwd.h"
#include "UiApi.h"

class UI_API NaaliGraphicsView : public QGraphicsView
{
    Q_OBJECT;

public:
    explicit NaaliGraphicsView(QWidget *parent);

    QImage *BackBuffer();
    bool IsViewDirty() const;
    void MarkViewUndirty();
    QRectF DirtyRectangle() const;

signals:
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
    void Resize(int newWidth, int newHeight);

private:
    QImage *backBuffer;
    QRectF dirtyRectangle;

    void drawBackground(QPainter *, const QRectF &);
    bool event(QEvent *event);
    void resizeEvent(QResizeEvent *e);
    void dragEnterEvent(QDragEnterEvent *e);
    void dragMoveEvent(QDragMoveEvent *e);
    void dropEvent(QDropEvent *e);

private slots:
    void HandleSceneChanged(const QList<QRectF> &rectangles);
};

#endif

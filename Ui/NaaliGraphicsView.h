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

public slots:
    void Resize(int newWidth, int newHeight);

private:
    QImage *backBuffer;
    QRectF dirtyRectangle;

    void drawBackground(QPainter *, const QRectF &);
    bool event(QEvent *event);
    void resizeEvent(QResizeEvent *e);

private slots:
    void HandleSceneChanged(const QList<QRectF> &rectangles);
};

#endif

#ifndef incl_RexLogic_TerrainLabel_h
#define incl_RexLogic_TerrainLabel_h

#include <Foundation.h>

#include <QLabel>

namespace RexLogic
{
    // This label will send a signal when some of mouse event has happend inside this label.
    class TerrainLabel: public QLabel
    {
        Q_OBJECT

    public:
        explicit TerrainLabel(QWidget *parent=0, Qt::WindowFlags f=0);
        explicit TerrainLabel(const QString &text, QWidget *parent=0, Qt::WindowFlags f=0);
        virtual ~TerrainLabel();

    signals:
        //! Emitted when mouse event has happened inside this label.
        void SendMouseEvent(QMouseEvent *ev);

    protected:
        void mousePressEvent(QMouseEvent *ev);
        void mouseMoveEvent(QMouseEvent *ev);
        void mouseReleaseEvent(QMouseEvent *ev);
    };
}

#endif

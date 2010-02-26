#ifndef incl_Environment_TerrainLabel_h
#define incl_Environment_TerrainLabel_h

#include <QLabel>

namespace Environment
{
    // This label will send a signal when some of mouse event has occured inside this label.
    //! \ingroup EnvironmentModuleClient.
    class TerrainLabel: public QLabel
    {
        Q_OBJECT

    public:
        explicit TerrainLabel(QWidget *parent=0, Qt::WindowFlags f=0);
        explicit TerrainLabel(const QString &text, QWidget *parent=0, Qt::WindowFlags f=0);
        virtual ~TerrainLabel();

    signals:
        //! Emitted when mouse event has triggered inside this label.
        void SendMouseEvent(QMouseEvent *ev);

    protected:
        void mousePressEvent(QMouseEvent *event);
        void mouseMoveEvent(QMouseEvent *event);
        void mouseReleaseEvent(QMouseEvent *event);
    };
}

#endif

// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_WorldBuildingModule_WorldObjectView_h
#define incl_WorldBuildingModule_WorldObjectView_h

#include <QLabel>
#include <QPointF>

class QMouseEvent;

namespace WorldBuilding
{
    
    class WorldObjectView: public QLabel
    {
        Q_OBJECT
    public:
        WorldObjectView(QWidget* widget=0);

    protected:
        void mousePressEvent ( QMouseEvent * e);
        void mouseReleaseEvent ( QMouseEvent * e);
        void mouseMoveEvent ( QMouseEvent * e);
        void wheelEvent ( QWheelEvent * e ); 

    signals:
        void RotateObject(qreal x, qreal y);
        void Zoom(qreal delta);

    private:
        bool left_mousebutton_pressed_;
        QPointF last_pos_;
        

    };

}

#endif

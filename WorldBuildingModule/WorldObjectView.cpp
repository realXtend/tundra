#include "StableHeaders.h"
#include "WorldObjectView.h"
#include <QMouseEvent>
#include <QDebug>
#include <QCursor>

#define PI 3.14159265

namespace WorldBuilding
{
    WorldObjectView::WorldObjectView(QWidget* widget):QLabel(widget),
        left_mousebutton_pressed_(false)
    {
    }

    void WorldObjectView::mousePressEvent ( QMouseEvent * e)
    {
        if(e->button()==Qt::LeftButton)
        {
            left_mousebutton_pressed_=true;
            QCursor cursor(Qt::SizeAllCursor);
            
           // if(!QApplication::overrideCursor)
                QApplication::setOverrideCursor(Qt::SizeAllCursor);
        }
    }
    void WorldObjectView::mouseReleaseEvent ( QMouseEvent * e)
    {
        if(e->button()==Qt::LeftButton)
        {
            left_mousebutton_pressed_=false;
            last_pos_.setX(0);
            last_pos_.setY(0);

            if(QApplication::overrideCursor)
                QApplication::restoreOverrideCursor();
            
        }
    }
    void WorldObjectView::mouseMoveEvent ( QMouseEvent * e)
    {
        
        if(left_mousebutton_pressed_)
        {

            QPointF current_pos = e->posF();

            if(!last_pos_.isNull())
            {
                QPointF movement = current_pos-last_pos_;
                emit RotateObject(2*PI*movement.x()/width(),2*PI*movement.y()/height());
            }
            last_pos_ = current_pos;

        }
    }
}
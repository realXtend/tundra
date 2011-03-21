//$ HEADER_NEW_FILE $

#include "StableHeaders.h"
#include "CameraView.h"

#include <QMouseEvent>
#include <QDebug>
#include <QCursor>

#define PI 3.14159265

namespace Camera
{
    CameraView::CameraView(QString title, QWidget* widget) :
        QLabel(widget),
        left_mousebutton_pressed_(false)
    {
        setWindowTitle(title);        
        setMinimumSize(QSize(300,300));
        setAlignment(Qt::AlignCenter);
        setText(title + " Camera View");
    }

    void CameraView::mousePressEvent(QMouseEvent *e)
    {
        if (e->button() == Qt::LeftButton)
        {
            left_mousebutton_pressed_ = true;
            if (!QApplication::overrideCursor())
                QApplication::changeOverrideCursor(Qt::SizeAllCursor);
            else
                QApplication::setOverrideCursor(Qt::SizeAllCursor);
            update_timer_.start();
        }
    }

    void CameraView::mouseReleaseEvent(QMouseEvent *e)
    {
        if (e->button() == Qt::LeftButton)
        {
            left_mousebutton_pressed_ = false;
            last_pos_.setX(0);
            last_pos_.setY(0);
            if (QApplication::overrideCursor())
                QApplication::restoreOverrideCursor();
        }
    }

    void CameraView::wheelEvent(QWheelEvent *e)
    {
        emit Zoom(e->delta());
    }

    void CameraView::mouseMoveEvent(QMouseEvent *e)
    {
        if (left_mousebutton_pressed_)
        {
            if (update_timer_.elapsed() < 20)
                return;
            QPointF current_pos = e->posF();
            if (!last_pos_.isNull())
            {
                QPointF movement = current_pos-last_pos_;
                emit Move(movement.x()/width(), movement.y()/height());
                
                //emit Move(2*PI*movement.x()/width(),2*PI*movement.y()/height());
            }
            last_pos_ = current_pos;
            update_timer_.start();
        }
    }
}
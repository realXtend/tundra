//$ HEADER_NEW_FILE $

#include "StableHeaders.h"
#include "CameraView.h"

#include <QMouseEvent>
#include <QDebug>
#include <QCursor>

#define PI 3.14159265

namespace Camera
{
    CameraWidget::CameraWidget(QString title, QWidget* widget) :
        QWidget(widget)
    {                     
        setupUi(this);        
        setWindowTitle(title);
        renderer = new CameraView(widgetRenderer);    
        verticalLayout_2->addWidget(renderer);
        connect(comboBoxCameras, SIGNAL(currentIndexChanged(const QString &)),this,  SLOT(SetWindowTitle(const QString &)));         
    }
    
    CameraWidget::~CameraWidget()
    {
        delete renderer;
    }

    void CameraWidget::SetWindowTitle(const QString &name)
    {
        
        if (this->parentWidget())                    
            dynamic_cast<QWidget*>(this->parentWidget())->setWindowTitle(name + " Camera");        
    }

    void CameraWidget::hideEvent(QHideEvent * event)
    {
        if (isHidden())
            emit WidgetHidden();
    }

    CameraView::CameraView(QWidget* widget) :
        QLabel(widget),
        left_mousebutton_pressed_(false)
    {              
        if (widget)
            widget->setMinimumSize(300,300);        
        setMinimumSize(300,300);
        setAlignment(Qt::AlignCenter);
        setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
        setText("Camera View");
    }

    void CameraView::mousePressEvent(QMouseEvent *e)
    {
        if (e->button() == Qt::LeftButton)
        {
            left_mousebutton_pressed_ = true;
            last_pos_= e->posF();
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
        emit Zoom(e->delta(), e->modifiers());
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
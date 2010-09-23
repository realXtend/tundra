// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DetachedWidgetController.h"
#include <QPushButton>
#include <QMouseEvent>
#include <QDebug>

DetachedWidgetController::DetachedWidgetController()
    :button_text_padding_(20.0f),
    mouse_drag_(false)
{
    setupUi(this);
    name->installEventFilter(this);
}

DetachedWidgetController::~DetachedWidgetController()
{

}

void DetachedWidgetController::AddButton(QPushButton *button)
{
   button->setParent(this);
   button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
   
   QBoxLayout* layout = dynamic_cast<QBoxLayout*>(this->buttongroup->layout());
   if(!layout)
       return;
   layout->insertWidget(0,button);
   button->show();
   this->update();
}
void DetachedWidgetController::DisableButtons(bool val)
{
    if(val)
    {
        for(int i=0; i<buttongroup->layout()->count();i++)
        {
            QPushButton * item =	dynamic_cast<QPushButton*>(buttongroup->layout()->itemAt(i)->widget());
            if(item)
            {

                item->hide();
            }
        }
    }
}

void DetachedWidgetController::SetText(QString text)
{
    name->setText(text);
    this->update();
}

void DetachedWidgetController::mousePressEvent ( QMouseEvent * event )
{
    prev_mouse_pos = event->globalPos();
    mouse_drag_ = true;
}
void DetachedWidgetController::mouseReleaseEvent ( QMouseEvent * event )
{
    mouse_drag_ = false;
}
void DetachedWidgetController::mouseMoveEvent ( QMouseEvent * event ) 
{
    QPoint pos = event->globalPos();
    QPoint move = pos - prev_mouse_pos;
    this->move(this->pos() +  move);
    prev_mouse_pos = pos;
}

bool DetachedWidgetController::eventFilter(QObject *obj, QEvent *event)
{
    
    QWidget::eventFilter(obj, event);
    return false;
}

// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "HoveringButtonsController.h"

#include <QPushButton>
#include <QMouseEvent>
#include <QDebug>

HoveringButtonsController::HoveringButtonsController() :
    text_padding_(10.0f)
{
    Ui::HoveringButtonsWidget::setupUi(this);
}

HoveringButtonsController::~HoveringButtonsController()
{

}

void HoveringButtonsController::ButtonPressed()
{
    qDebug() << ">> HoveringWidget - Debug poke!";
}

void HoveringButtonsController::ForwardMouseClickEvent(float x, float y)
{ 
    // To widgetspace
    float x_widgetspace = x*(float)this->width();
    float y_widgetspace = y*(float)this->height();
    QPoint pos(x_widgetspace,y_widgetspace);

    QMouseEvent e(QEvent::MouseButtonPress, pos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(this, &e);
    
    // Iterate through buttons and check if they are pressed 
    for(int i = 0; i< layout()->count();i++)
    {
        QLayoutItem* item = layout()->itemAt(i);
        QAbstractButton *button = dynamic_cast<QAbstractButton*>(item->widget());
        if(button)
        {
            if(button->rect().contains(button->mapFromParent(pos)))
            {
                QMouseEvent e(QEvent::MouseButtonPress, pos - button->pos(),pos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
                QApplication::sendEvent(button, &e);
            }
        }
    }
}

void HoveringButtonsController::AddButton(QPushButton *button)
{
   button->setParent(this);
   button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
   button->setMaximumHeight(30);
   button->setMaximumWidth(60);
   
   QFont font(button->font());
   QRect rect = button->fontMetrics().tightBoundingRect(button->text());
   qreal scale = (button->maximumWidth() - text_padding_)/rect.width();
   font.setPointSize(font.pointSizeF() * scale);
   button->setFont(font);

   QBoxLayout* layout = dynamic_cast<QBoxLayout*>(this->layout());
   if (!layout)
       return;

   layout->insertWidget(0,button);
   QObject::connect(button, SIGNAL(pressed()), this, SLOT(ButtonPressed())); 
   button->show();
}

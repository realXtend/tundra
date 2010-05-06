#include "StableHeaders.h"
#include "HoveringWidgetController.h"
#include <QPushButton>
#include <QMouseEvent>
#include <QDebug>

namespace RexLogic
{

    HoveringWidgetController::HoveringWidgetController()
    :text_padding_(100.0f)
    {
        Ui::HoveringWidget::setupUi(this);
    }

    HoveringWidgetController::~HoveringWidgetController()
    {

    }

    void HoveringWidgetController::SetText(const QString &text)
    {
        Real l_width = label->fontMetrics().width(text);
        QFont font = label->font();
        Real scale = (this->width()- text_padding_) / l_width;
        if(scale>0)
            font.setPointSizeF(font.pointSizeF()*scale);
        label->setFont(font);
        label->setText(text);
    }
    void HoveringWidgetController::ButtonPressed()
    {
        qDebug() << "clicked!";
    }

    void HoveringWidgetController::ForwardMouseClickEvent(Real x, Real y)
    {
        //To widgetspace
        Real x_widgetspace = x*(Real)this->width();
        Real y_widgetspace = y*(Real)this->height();

        QPoint pos(x_widgetspace,y_widgetspace);

        QMouseEvent e(QEvent::MouseButtonPress, pos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
        QApplication::sendEvent(this, &e);
        //Iterate through buttons and check if they are pressed 
        for(int i = 0; i< buttons_layout->count();i++)
        {
            QLayoutItem* item = buttons_layout->itemAt(i);
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

    void HoveringWidgetController::ShowButtons()
    {
        for(int i = 0; i< buttons_layout->count();i++)
        {
            QLayoutItem* item = buttons_layout->itemAt(i);
            QAbstractButton *button = dynamic_cast<QAbstractButton*>(item->widget());
            if(button)
            {
                button->show();
            }
        }
    }

    void HoveringWidgetController::HideButtons()
    {
                for(int i = 0; i< buttons_layout->count();i++)
        {
            QLayoutItem* item = buttons_layout->itemAt(i);
            QAbstractButton *button = dynamic_cast<QAbstractButton*>(item->widget());
            if(button)
            {
                button->hide();
            }
        }
    }

    void HoveringWidgetController::AddButton(QPushButton *button)
    {
       button->setParent(this);
       button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
       button->setMaximumHeight(150);
       button->setMaximumWidth(300);
       QFont font(button->font());
       QRect rect = button->fontMetrics().tightBoundingRect(button->text());

       qreal scale = (button->maximumWidth() - text_padding_)/rect.width();

       font.setPointSize(font.pointSizeF() * scale);
       button->setFont(font);
       this->buttons_layout->insertWidget(1,button);
       QObject::connect(button, SIGNAL(pressed()), this, SLOT(ButtonPressed()));
    }
}

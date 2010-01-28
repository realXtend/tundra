#include "StableHeaders.h"
#include "ConsoleProxyWidget.h"

#include <QGraphicsScene>
#include <QWidget>
#include <QKeyEvent>
#include <QRect>
#include <QEvent>
#include <QGraphicsSceneResizeEvent>

namespace CoreUi
{

    ConsoleProxyWidget::ConsoleProxyWidget(QWidget *widget)
        : QGraphicsProxyWidget(0, Qt::Widget),
        con_relative_height_(0.5)
       
    {
        setWidget(widget);
    }

    ConsoleProxyWidget::~ConsoleProxyWidget(void)
    {
    }

    void ConsoleProxyWidget::setConsoleRelativeHeight(qreal height)
    {
        //check if clamping is needed
        if(height < 0 || height > 1)
            height /= height;
        con_relative_height_ = height;
    }
    

    void ConsoleProxyWidget::keyPressEvent(QKeyEvent *e)
        {
            QGraphicsProxyWidget::keyPressEvent(e);
            //Right now we have hardcoded the "console button" in 2 places. This should be fetched from somewhere else
            if(e->key() == Qt::Key_F1)
            {
                emit TConsoleButtonPressed();
            }
        }
}

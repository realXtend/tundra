// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "ConsoleProxyWidget.h"

#include <QWidget>
#include "MemoryLeakCheck.h"

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

    void ConsoleProxyWidget::SetConsoleRelativeHeight(qreal height)
    {
        if (height < 0 || height > 1)
            height /= height;
        con_relative_height_ = height;
    }
}

// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "UiProxyStyle.h"

#include <QStyleOptionTitleBar>

#include "MemoryLeakCheck.h"

namespace UiServices
{
    //TODO tweak the following values for other windowing systems

    int UiProxyStyle::pixelMetric(PixelMetric metric, const QStyleOption *option, const QWidget *widget) const
    {
#ifndef Q_WS_X11
        switch (metric)
        {
            case QStyle::PM_TitleBarHeight:
                return 22; // Decrease title bar height
            case QStyle::PM_MdiSubWindowFrameWidth:
                return 5; // Smaller dialog frame
            case QStyle::PM_SmallIconSize:
                const QStyleOptionTitleBar *title_bar_option = qstyleoption_cast<const QStyleOptionTitleBar *>(option);
                if (title_bar_option)
                    return 0; // Disable dialog title bar icon
        }
#endif 
        return QProxyStyle::pixelMetric(metric, option, widget);
    }

    QRect UiProxyStyle::subControlRect(ComplexControl control, const QStyleOptionComplex *option, SubControl subControl, const QWidget *widget) const
    {
#ifndef Q_WS_X11
        // We are only interested in modifying the TitleBar subcontrol rects
        if (control == QStyle::CC_TitleBar)
        {
            QRect rect;
            switch (subControl)
            {
                // Reposition the window title
                case QStyle::SC_TitleBarLabel:
                    rect = QProxyStyle::subControlRect(control, option, subControl, widget);
                    rect.setLeft(5);
                    rect.setTop(-5);
                    return rect;
                // Reposition/resize the close button
                case QStyle::SC_TitleBarCloseButton:
                    rect = QProxyStyle::subControlRect(control, option, subControl, widget);
                    rect.setRight(rect.right()-5);
                    rect.setTop(6);
                    return rect;
            }
        }
#endif
        return QProxyStyle::subControlRect(control, option, subControl, widget);
    }
}

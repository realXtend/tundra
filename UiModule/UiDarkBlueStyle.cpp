// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "UiDarkBlueStyle.h"

#include <QWebView>

#include "MemoryLeakCheck.h"

#define CL_MIN(a,b) (a)<(b) ? (a):(b) // remove this when it is working

namespace UiServices
{

    UiDarkBlueStyle::UiDarkBlueStyle ()
    {
        default_palette_ = QApplication::palette();
    }

    void UiDarkBlueStyle::polish(QPalette &palette)
    {     
        QColor lightgray(209, 212, 214);
        QColor highlight_lightgray(241, 241, 241);
        QColor base(42, 45, 50);
        QColor paletteColor(39, 41, 44);
        QColor white(236, 236, 236);
        QColor slightlyOpaqueBlack(0, 0, 0, 60);
        QColor baseColor(63, 72, 86);
        QColor alternateBaseColor(49, 54, 62);
        QColor highlightedText(19, 20, 20);

        QPixmap backgroundImage("./data/ui/images/styles/darkblue_bg.png");
        QPixmap buttonImage("./data/ui/images/styles/darkblue_button.png");

        QPixmap midImage = buttonImage;

        QPainter painter;
        painter.begin(&midImage);
        painter.setPen(Qt::NoPen);
        painter.fillRect(midImage.rect(), slightlyOpaqueBlack);
        painter.end();

        palette = QPalette(paletteColor);

        palette.setBrush(QPalette::BrightText, Qt::white);
        palette.setBrush(QPalette::WindowText, white);
        palette.setBrush(QPalette::Base, baseColor);
        palette.setBrush(QPalette::AlternateBase, alternateBaseColor);
        palette.setBrush(QPalette::Highlight, highlight_lightgray);
        palette.setBrush(QPalette::ButtonText, white);
        palette.setBrush(QPalette::HighlightedText, highlightedText);  

        setTexture(palette, QPalette::Button, buttonImage);     
        setTexture(palette, QPalette::Mid, midImage);
        setTexture(palette, QPalette::Window, backgroundImage);

        QBrush brush = palette.background();
        brush.setColor(Qt::lightGray);

        palette.setBrush(QPalette::Disabled, QPalette::WindowText, brush);
        palette.setBrush(QPalette::Disabled, QPalette::Text, brush);
        palette.setBrush(QPalette::Disabled, QPalette::ButtonText, brush);
        palette.setBrush(QPalette::Disabled, QPalette::Base, brush);
        palette.setBrush(QPalette::Disabled, QPalette::Button, brush);
        palette.setBrush(QPalette::Disabled, QPalette::Mid, brush);


    }

    void UiDarkBlueStyle::polish(QWidget *widget)
    {        
        if (qobject_cast<QPushButton *>(widget)
            || qobject_cast<QComboBox *>(widget))
            widget->setAttribute(Qt::WA_Hover, true); 

        if (qobject_cast<QScrollArea *>(widget))
            widget->setBackgroundRole(QPalette::Base);

        if (qobject_cast<QWebView *>(widget))
        {
            widget->setPalette(default_palette_);
        }

    }

    void UiDarkBlueStyle::unpolish(QWidget *widget)
    {
        if (qobject_cast<QPushButton *>(widget)
            || qobject_cast<QComboBox *>(widget))
            widget->setAttribute(Qt::WA_Hover, false);
    }

    int UiDarkBlueStyle::pixelMetric(PixelMetric metric,
        const QStyleOption *option,
        const QWidget *widget) const
    {
#ifndef Q_WS_X11
        switch (metric)
        {
        case QStyle::PM_TitleBarHeight:
            return 25; // Decrease title bar height
        case QStyle::PM_MdiSubWindowFrameWidth:
            return 5; // Smaller dialog frame            
        case QStyle::PM_IconViewIconSize:
            return 0;
        case QStyle::PM_SmallIconSize:
            const QStyleOptionTitleBar *title_bar_option = qstyleoption_cast<const QStyleOptionTitleBar *>(option);
            if (title_bar_option)
                return 0; // Disable dialog title bar icon
        }
#endif 
        return QCleanlooksStyle::pixelMetric(metric, option, widget);
    }

    int UiDarkBlueStyle::styleHint(StyleHint hint, const QStyleOption *option,
        const QWidget *widget,
        QStyleHintReturn *returnData) const
    {
        switch (hint) {
     case SH_DitherDisabledText:
         return int(false);
     case SH_EtchDisabledText:
         return int(true);
     default:
         return QCleanlooksStyle::styleHint(hint, option, widget, returnData);
        }
    }

    void UiDarkBlueStyle::drawComplexControl(ComplexControl control, const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const
    {

        switch (control)
        {
        case QStyle::CC_TitleBar:
            {
                const int buttonMargin = 0;
                painter->save();

                const QStyleOptionTitleBar *myTitleBar =
                    qstyleoption_cast<const QStyleOptionTitleBar *>(option);
                if (myTitleBar) {

                    bool active = (myTitleBar->titleBarState & State_Active);

                    QRect textRect = proxy()->subControlRect(CC_TitleBar, myTitleBar, SC_TitleBarLabel, widget);
                    QFont font("facetextrabold", 10, 25, false);
                    font.setCapitalization(QFont::AllUppercase);
                    font.setStyleStrategy(QFont::PreferAntialias);
                    font.setLetterSpacing(QFont::AbsoluteSpacing, 1);

                    painter->setRenderHint(QPainter::Antialiasing, true);
                    painter->setFont(font);
                    painter->setPen(myTitleBar->palette.text().color());

                    bool tooWide = (QFontMetrics(font).width(myTitleBar->text) > textRect.width());
                    QTextOption option((tooWide ? Qt::AlignLeft : Qt::AlignHCenter) | Qt::AlignVCenter);
                    option.setWrapMode(QTextOption::NoWrap);
                    painter->setPen(QColor(232, 229, 228));                    
                    painter->drawText(textRect, myTitleBar->text, option);

                    if ((myTitleBar->subControls & SC_TitleBarCloseButton) && (myTitleBar->titleBarFlags & Qt::WindowSystemMenuHint)) {
                        QRect closeButtonRect = proxy()->subControlRect(CC_TitleBar, myTitleBar, SC_TitleBarCloseButton, widget);
                        if (closeButtonRect.isValid()) {
                            bool hover = (myTitleBar->activeSubControls & SC_TitleBarCloseButton) && (myTitleBar->state & State_MouseOver);
                            bool sunken = (myTitleBar->activeSubControls & SC_TitleBarCloseButton) && (myTitleBar->state & State_Sunken);

                            QRect closeIconRect = closeButtonRect.adjusted(buttonMargin, buttonMargin, -buttonMargin, -buttonMargin);
                            QPixmap buttonImage;
                            if (hover)
                            {
                                buttonImage.load("./data/ui/images/styles/darkblue_closebuttonhover.png");
                            }                                
                            else
                            {
                                buttonImage.load("./data/ui/images/styles/darkblue_closebutton.png");
                            }                            
                            painter->save();
                            proxy()->drawItemPixmap(painter, closeIconRect, Qt::AlignCenter, buttonImage);
                            painter->restore();
                        }
                    }

                    if ((myTitleBar->subControls & SC_TitleBarSysMenu) && (myTitleBar->titleBarFlags & Qt::WindowSystemMenuHint)) {
                        bool hover = (myTitleBar->activeSubControls & SC_TitleBarSysMenu) && (myTitleBar->state & State_MouseOver);
                        bool sunken = (myTitleBar->activeSubControls & SC_TitleBarSysMenu) && (myTitleBar->state & State_Sunken);

                        QRect iconRect = proxy()->subControlRect(CC_TitleBar, myTitleBar, SC_TitleBarSysMenu, widget);
                        if (!myTitleBar->icon.isNull()) {
                            myTitleBar->icon.paint(painter, QRect(0, 0, 0, 0));
                        } else {
                            myTitleBar->icon.paint(painter, QRect(0, 0, 0, 0));
                            painter->save();
                            painter->restore();
                        }
                    }

                    painter->restore();                    
                }
            }
            break;
        default:
            QCleanlooksStyle::drawComplexControl(control, option, painter, widget);
        }
    }

    void UiDarkBlueStyle::drawPrimitive(PrimitiveElement element,
        const QStyleOption *option,
        QPainter *painter,
        const QWidget *widget) const
    {
        switch (element) {
     case PE_PanelButtonCommand:
         {
             int delta = (option->state & State_MouseOver) ? 64 : 0;
             QColor slightlyOpaqueBlack(0, 0, 0, 63);
             QColor semiTransparentWhite(255, 255, 255, 127 + delta);
             QColor semiTransparentBlack(0, 0, 0, 127 - delta);

             int x, y, width, height;
             option->rect.getRect(&x, &y, &width, &height);

             QPainterPath roundRect = roundRectPath(option->rect);
             int radius = qMin(width, height) / 5;

             QBrush brush;
             bool darker;

             const QStyleOptionButton *buttonOption = qstyleoption_cast<const QStyleOptionButton *>(option);
             if (buttonOption && (buttonOption->features & QStyleOptionButton::Flat)) {
                 brush = option->palette.background();
                 darker = (option->state & (State_Sunken | State_On));
             } else {
                 if (option->state & (State_Sunken | State_On)) {
                     brush = option->palette.mid();
                     darker = !(option->state & State_Sunken);
                 } else {
                     brush = option->palette.button();
                     darker = false;
                 }
             }

             QFont font("facetextrabold", 8, 0, false);
             if (widget)
             {
                qreal w_psize = widget->font().pointSize();
                if (w_psize > 8)
                    font.setPointSize(w_psize);
             }
             font.setCapitalization(QFont::AllUppercase);
             font.setStyleStrategy(QFont::PreferAntialias);
             font.setLetterSpacing(QFont::AbsoluteSpacing, 1);
             painter->setRenderHint(QPainter::Antialiasing, true);
             painter->setFont(font);

             painter->save();
             painter->setRenderHint(QPainter::Antialiasing, true);
             painter->fillPath(roundRect, brush);
             if (darker)
                 painter->fillPath(roundRect, slightlyOpaqueBlack);

             int x1 = x;
             int x2 = x + radius;
             int x3 = x + width - radius;
             int x4 = x + width;

             if (option->direction == Qt::RightToLeft) {
                 qSwap(x1, x4);
                 qSwap(x2, x3);
             }

             QPolygon topHalf;
             topHalf << QPoint(x1, y)
                 << QPoint(x4, y)
                 << QPoint(x3, y + radius)
                 << QPoint(x2, y + height - radius)
                 << QPoint(x1, y + height);

             QPen topPen(semiTransparentWhite, 1);
             QPen bottomPen(semiTransparentBlack, 1);

             painter->setClipPath(roundRect);
             painter->setClipRegion(topHalf, Qt::IntersectClip);
             painter->setPen(topPen);
             painter->drawPath(roundRect);

             QPolygon bottomHalf = topHalf;
             bottomHalf[0] = QPoint(x4, y + height);

             painter->setClipPath(roundRect);
             painter->setClipRegion(bottomHalf, Qt::IntersectClip);
             painter->setPen(bottomPen);
             painter->drawPath(roundRect);

             painter->setPen(Qt::NoPen);
             painter->setClipping(false);
             painter->drawPath(roundRect);

             painter->restore();
         }
         break; 
     case PE_Frame:
         {
             painter->save();
             painter->setPen(QColor(209, 212, 214));
             painter->setBrush(QColor(42, 45, 50));
             painter->drawRect(option->rect.adjusted(0, 0, -1, -1));
             painter->restore();
         }
     default:
         QCleanlooksStyle::drawPrimitive(element, option, painter, widget);
        }
    }

    void UiDarkBlueStyle::drawControl(ControlElement element,
        const QStyleOption *option,
        QPainter *painter,
        const QWidget *widget) const
    {
        switch (element) {
     case CE_PushButtonLabel:
         {
             QStyleOptionButton myButtonOption;
             const QStyleOptionButton *buttonOption =
                 qstyleoption_cast<const QStyleOptionButton *>(option);
             if (buttonOption) {
                 myButtonOption = *buttonOption;
                 if (myButtonOption.palette.currentColorGroup()
                     != QPalette::Disabled) {
                         if (myButtonOption.state & (State_Sunken | State_On)) {
                             myButtonOption.palette.setBrush(QPalette::ButtonText,
                                 myButtonOption.palette.brightText());
                         }
                 }
             }
             QCleanlooksStyle::drawControl(element, &myButtonOption, painter, widget);
         }
         break;     
     default:
         QCleanlooksStyle::drawControl(element, option, painter, widget);
        }
    }

    void UiDarkBlueStyle::setTexture(QPalette &palette, QPalette::ColorRole role,
        const QPixmap &pixmap)
    {
        for (int i = 0; i < QPalette::NColorGroups; ++i) {
            QColor color = palette.brush(QPalette::ColorGroup(i), role).color();
            palette.setBrush(QPalette::ColorGroup(i), role, QBrush(color, pixmap));
        }
    }

    QPainterPath UiDarkBlueStyle::roundRectPath(const QRect &rect)
    {
        int radius = qMin(rect.width(), rect.height()) / 5;
        int diam = 2 * radius;

        int x1, y1, x2, y2;
        rect.getCoords(&x1, &y1, &x2, &y2);

        QPainterPath path;
        path.moveTo(x2, y1 + radius);
        path.arcTo(QRect(x2 - diam, y1, diam, diam), 0.0, +90.0);
        path.lineTo(x1 + radius, y1);
        path.arcTo(QRect(x1, y1, diam, diam), 90.0, +90.0);
        path.lineTo(x1, y2 - radius);
        path.arcTo(QRect(x1, y2 - diam, diam, diam), 180.0, +90.0);
        path.lineTo(x1 + radius, y2);
        path.arcTo(QRect(x2 - diam, y2 - diam, diam, diam), 270.0, +90.0);
        path.closeSubpath();
        return path;
    }

    QRect UiDarkBlueStyle::subControlRect(ComplexControl control, const QStyleOptionComplex *option, SubControl subControl, const QWidget *widget) const
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
                rect = QCleanlooksStyle::subControlRect(control, option, subControl, widget);                
                rect.setTop(-2);
                return rect;
                // Reposition/resize the close button
            case QStyle::SC_TitleBarCloseButton:
                rect = QCleanlooksStyle::subControlRect(control, option, subControl, widget);
                rect.setRight(rect.right()-3);
                rect.setTop(6);
                return rect;      
            }
        }
#endif
        return QCleanlooksStyle::subControlRect(control, option, subControl, widget);
    }

}

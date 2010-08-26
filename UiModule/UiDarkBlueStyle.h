// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_UiDarkBlueStyle_h
#define incl_UiModule_UiDarkBlueStyle_h

#include <QCleanlooksStyle>
#include <QPalette>
#include <QPainter>
#include <QPainterPath>
#include <QStyleOptionTitleBar>
#include <QRect>

namespace UiServices
{
    //! Custom QCleanlooksStyle for modifying the paint information of scene widgets 
    class UiDarkBlueStyle : public QCleanlooksStyle
    {
        
    Q_OBJECT

    public:
        UiDarkBlueStyle();

        void polish(QPalette &palette);
        void polish(QWidget *widget);
        void unpolish(QWidget *widget);

        int pixelMetric(PixelMetric metric, const QStyleOption *option, const QWidget *widget) const;
        int styleHint(StyleHint hint, const QStyleOption *option, const QWidget *widget, QStyleHintReturn *returnData) const;
        QSize sizeFromContents(ContentsType type, const QStyleOption *option, const QSize &contentsSize, const QWidget *widget = 0) const;

        void drawPrimitive(PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const;
        void drawControl(ControlElement control, const QStyleOption *option, QPainter *painter, const QWidget *widget) const;
        void drawComplexControl(ComplexControl control, const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const;        
        QRect subControlRect(ComplexControl control, const QStyleOptionComplex *option, SubControl subControl, const QWidget *widget = 0) const;

    private:
        static void setTexture(QPalette &palette, QPalette::ColorRole role, const QPixmap &pixmap);
        static QPainterPath roundRectPath(const QRect &rect);

        QPalette default_palette_;

    };

}

#endif // incl_UiModule_UiDarkBlueStyle_h
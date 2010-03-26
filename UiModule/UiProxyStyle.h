// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_UiProxyStyle_h
#define incl_UiModule_UiProxyStyle_h

#include <QProxyStyle>

namespace UiServices
{
    //! Custom QProxyStyle for modifying the paint information of scene widgets 
    class UiProxyStyle : public QProxyStyle
    {

    public:
        int pixelMetric(PixelMetric metric, const QStyleOption *option = 0, const QWidget *widget = 0) const;
        QRect subControlRect(ComplexControl control, const QStyleOptionComplex *option, SubControl subControl, const QWidget *widget = 0) const;

    };
}

#endif // incl_Foundation_UiProxyStyle_h
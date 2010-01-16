// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Foundation_QNaaliProxyStyle_h
#define incl_Foundation_QNaaliProxyStyle_h

#include <QProxyStyle>

namespace Foundation
{
    class QNaaliProxyStyle : public QProxyStyle
    {

    public:
        int pixelMetric(PixelMetric metric, const QStyleOption *option = 0, const QWidget *widget = 0) const;
        QRect subControlRect(ComplexControl control, const QStyleOptionComplex *option, SubControl subControl, const QWidget *widget = 0) const;

    };
}

#endif // incl_Foundation_QNaaliProxyStyle_h
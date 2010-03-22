// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_BackdropWidget_h
#define incl_UiModule_BackdropWidget_h

#include <QGraphicsProxyWidget>
#include "ui_BackdropWidget.h"

namespace CoreUi
{
    class BackdropWidget : public QGraphicsProxyWidget, private Ui::BackdropWidget
    {
    
    Q_OBJECT

    public:
        BackdropWidget();

    public slots:
        void SetContentWidth(qreal width);
        void SetWidgetWidth(qreal width);

        qreal GetContentWidth();
        qreal GetContentHeight();
        qreal GetWidgetWidth();

    private:
        QWidget *internal_widget_;

    };
}

#endif
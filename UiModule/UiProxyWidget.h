// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_UiProxyWidget_h
#define incl_UiModule_UiProxyWidget_h

#include "UiModuleApi.h"
#include "UiWidgetProperties.h"

#include <QtGui>
#include <QtCore>

namespace UiServices
{
    /*************** UiProxyWidget : QGraphicsProxyWidget -> QGraphicsWidget -> QGraphicsItem ***************/

    class UI_MODULE_API UiProxyWidget : public QGraphicsProxyWidget
    {
    
    Q_OBJECT

    public:
        UiProxyWidget(QWidget *widget, const UiWidgetProperties &in_widget_properties);
        ~UiProxyWidget();

        //! QGraphicsProxyWidget override functions
        void showEvent(QShowEvent *show_event);
        void hideEvent(QHideEvent *hide_event);

        //! Getters
        UiWidgetProperties getWidgetProperties();

    private:
        void InitAnimations();

        UiWidgetProperties widget_properties_;
        QTimeLine *show_timeline_;
        QGraphicsItemAnimation *show_animation_;

    private slots:
        void AnimationStep(int step);

    signals:
        void Visible(bool);
    };

}

#endif // incl_UiModule_UiProxyWidget_h

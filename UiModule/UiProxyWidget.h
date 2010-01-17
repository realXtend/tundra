// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_UiProxyWidget_h
#define incl_UiModule_UiProxyWidget_h

#include "UiModuleApi.h"
#include "UiWidgetProperties.h"

#include <QGraphicsProxyWidget>

class QTimeLine;
class QGraphicsItemAnimation;

namespace CoreUi
{
    class MainPanelButton;
}

namespace UiServices
{
    /*************** UiProxyWidget : QGraphicsProxyWidget -> QGraphicsWidget -> QGraphicsItem ***************/

    class UI_MODULE_API UiProxyWidget : public QGraphicsProxyWidget
    {
        Q_OBJECT

    public:
        UiProxyWidget(QWidget *widget, const UiWidgetProperties &in_widget_properties);
        ~UiProxyWidget();

        //! @return UiWidgetProperties.
        UiWidgetProperties getWidgetProperties() const { return widget_properties_; }
        void SetControlButton(CoreUi::MainPanelButton *control_button);

    protected:
        //! QGraphicsProxyWidget override functions
        void showEvent(QShowEvent *show_event);
        void hideEvent(QHideEvent *hide_event);
        void closeEvent(QCloseEvent *close_event);
        void focusInEvent(QFocusEvent *focus_event);
        void focusOutEvent(QFocusEvent *focus_event);
        QVariant itemChange(GraphicsItemChange change, const QVariant &value);

    private:
        void InitAnimations();

        UiWidgetProperties widget_properties_;
        CoreUi::MainPanelButton *control_button_;
        QTimeLine *show_timeline_;

    private slots:
        void AnimationStep(qreal step);

    signals:
        void Closed();
        void Visible(bool);
        void BringToFrontRequest(UiProxyWidget*);
    };

}

#endif // incl_UiModule_UiProxyWidget_h

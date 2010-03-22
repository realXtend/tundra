// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_UiProxyWidget_h
#define incl_UiModule_UiProxyWidget_h

#include "UiModuleApi.h"
#include "UiWidgetProperties.h"

#include <QGraphicsProxyWidget>

class QTimeLine;
class QParallelAnimationGroup;
class QPropertyAnimation;

namespace UiServices
{
    /*************** UiProxyWidget : QGraphicsProxyWidget -> QGraphicsWidget -> QGraphicsItem ***************/

    class UI_MODULE_API UiProxyWidget : public QGraphicsProxyWidget
    {
        Q_OBJECT

    public:
        //! Constructor.
        //! \param widget
        //! \param in_widget_properties
        UiProxyWidget(QWidget *widget, const UiWidgetProperties &in_widget_properties);

        //! Destructor.
        ~UiProxyWidget();

        //! Get this proxys widget properties
        /// \return UiWidgetProperties.
        UiWidgetProperties GetWidgetProperties() const { return widget_properties_; }

        //! Set new opacity
        void SetUnfocusedOpacity(int new_opacity);

        //! Set new show animation speed
        void SetShowAnimationSpeed(int new_speed);

        //! Brings to front in the scene, sets focus and shows this proxy widget.
        //! \todo Seems that isn't working properly.
        void BringToFront();

    public slots:
        void AnimatedHide();

    protected:
        //! QGraphicsProxyWidget override functions
        void showEvent(QShowEvent *show_event);
        void hideEvent(QHideEvent *hide_event);
        void closeEvent(QCloseEvent *close_event);
        void focusInEvent(QFocusEvent *focus_event);
        void focusOutEvent(QFocusEvent *focus_event);
        QVariant itemChange(GraphicsItemChange change, const QVariant &value);

    private slots:
        void InitWidgetAndProxy(QWidget *widget);
        void InitEffectsAndAnimations();
        void FinishHide();

    private:
        UiWidgetProperties widget_properties_;
        QParallelAnimationGroup *animations_;
        QPropertyAnimation *fade_animation_;
        qreal unfocus_opacity_;
        bool show_animation_enabled_;

    signals:
        void Closed();
        void Visible(bool);
        void BringProxyToFrontRequest(UiProxyWidget*);
    };

}

#endif // incl_UiModule_UiProxyWidget_h

// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_ControlProxyWidget_h
#define incl_UiModule_ControlProxyWidget_h

#include <QGraphicsProxyWidget>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QLabel>
#include <QPushButton>

#include "UiModuleFwd.h"
//#include "InfoCard.h"
//#include "ActionProxyWidget.h"

namespace Ether
{
    namespace View
    {
        class ControlProxyWidget : public QGraphicsProxyWidget
        {

        Q_OBJECT

        public:

            enum ControlType
            {
                CardControl,
                ActionControl,
                AddRemoveControl
            };

            enum LayoutDirection
            {
                TopToBottom,
                BottomToTop,
                RightToLeft,
                LeftToRight,
                NoneDirection
            };

            enum Position
            {
                TopCenter,
                TopLeft,
                TopRight,
                BottomCenter,
                BottomLeft,
                BottomRight,
                LeftCenter,
                RightCenter,
                NonePosition
            };

            ControlProxyWidget(ControlType type, LayoutDirection direction, QString text = QString());
            void EmitActionRequest(QString type);

        public slots:
            void UpdateGeometry(QRectF rect, qreal scale, bool do_fade);
            void UpdateContollerCard(InfoCard *new_card);
            void SetActionWidget(View::ActionProxyWidget *action_widget) { action_widget_ = action_widget; }
            void SuppressButtons(bool suppress);

        private slots:
            void InitCardWidgets();
            void InitActionWidgets();
            void InitAddRemoveControl();

            void ControlledWidgetStopped();

            void RegisterHandler();
            void InfoHandler();
            void EditHandler();
            void AddHandler();
            void RemoveHandler();
            void ExitHandler();
            void ConnectHandler();
            void HelpHandler();

        private:
            ControlType type_;
            LayoutDirection direction_;
            Position lock_pos_;

            QMap<QWidget*, Position> widget_map_;

            QString text_;
            QLabel *text_label_;
            QWidget *parent_;

            View::InfoCard *controlled_card_;
            ActionProxyWidget *action_widget_;

            QParallelAnimationGroup *all_action_widget_animations_;
            QPropertyAnimation *fade_animation_;
            QPropertyAnimation *scale_animation_;
            QPropertyAnimation *move_animation_;

            QRectF last_scene_rect_;

            bool suppress_buttons_;

        signals:
            void ActionRequest(QString);
        };
    }
}

#endif // CONTROLPROXYWIDGET_H

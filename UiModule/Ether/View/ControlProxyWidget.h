// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_ControlProxyWidget_h
#define incl_UiModule_ControlProxyWidget_h

#include <QGraphicsProxyWidget>
#include <QPropertyAnimation>
#include <QLabel>
#include <QPushButton>

#include "InfoCard.h"
#include "ActionProxyWidget.h"

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
                ActionControl
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

            ControlProxyWidget(ControlType type, LayoutDirection direction, QString text);
            void EmitActionRequest(QString type);

        public slots:
            void UpdateGeometry(QRectF rect, qreal scale);
            void UpdateContollerCard(InfoCard *new_card);

            void SetActionWidget(View::ActionProxyWidget *action_widget) { action_widget_ = action_widget; }

        private slots:
            void InitCardWidgets();
            void InitActionWidgets();
            void ControlledWidgetStopped();

            void InfoHandler();
            void QuestionHandler();
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

        signals:
            void ActionRequest(QString);
        };
    }
}

#endif // CONTROLPROXYWIDGET_H

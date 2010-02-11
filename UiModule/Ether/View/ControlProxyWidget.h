// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_ControlProxyWidget_h
#define incl_UiModule_ControlProxyWidget_h

#include <QGraphicsProxyWidget>
#include <QPropertyAnimation>
#include <QLabel>
#include <QPushButton>

#include "InfoCard.h"

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
                None

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
                RightCenter
            };

            ControlProxyWidget(ControlType type, LayoutDirection direction, QString text);

        public slots:
            void UpdateGeometry(QRectF rect);
            void UpdateContollerCard(InfoCard *new_card);

        private slots:
            void InitCardWidgets();
            void InitActionWidgets();
            void ControlledWidgetStopped();

            void InfoHandler();
            void QuestionHandler();
            void AddHandler();
            void RemoveHandler();

        private:
            ControlType type_;
            LayoutDirection direction_;
            Position lock_pos_;

            QMap<QWidget*, Position> widget_map_;

            QString text_;
            QLabel *text_label_;
            QWidget *parent_;

            InfoCard *controlled_card_;
        };
    }
}

#endif // CONTROLPROXYWIDGET_H

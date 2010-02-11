// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_InfoCard_h
#define incl_UiModule_InfoCard_h

#include <QGraphicsWidget>
#include <QSequentialAnimationGroup>
#include <QPropertyAnimation>
#include <QPainter>
#include <QPixmap>
#include <QUuid>

namespace Ether
{
    namespace View
    {
        class InfoCard : public QGraphicsWidget
        {

        Q_OBJECT

        public:
            enum ArragementType { TopToBottom, BottomToTop };

            InfoCard(ArragementType type, QRectF bounding_rect, QUuid mapping_id, QString title, QString pixmap_path);

            QRectF boundingRect() const;
            void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

        public slots:
            void IsActiveItem(bool active);
            QString title() { return title_; }
            ArragementType arragementType() { return type_; }

            void SetMoveAnimationPointer(QPropertyAnimation *ptr) { move_animation_pointer_ = ptr; }
            QPropertyAnimation *GetMoveAnimationPointer() { return move_animation_pointer_; }

        private slots:
            void InitPaintHelpers(QString pixmap_path);
            void InitDecorations();

        protected:
            ArragementType type_;

            QUuid id_;
            QString title_;
            QString pixmap_path_;

            QPixmap pixmap_;
            QRectF bounding_rectf_;
            QRectF title_rectf_;

            QFont font_;
            QPen pen_;
            QBrush bg_brush_;

            QSequentialAnimationGroup *active_animations_;
            QPropertyAnimation *blur_animation_;
            QPropertyAnimation *jump_animation_;

            QPropertyAnimation *move_animation_pointer_;

        signals:
            void Clicked(View::InfoCard *clicked_item);
        };
    }
}

#endif // INFOCARD_H

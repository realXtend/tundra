// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_WorldMap_ImageItem_h
#define incl_WorldMap_ImageItem_h

#include <QtCore>
#include <QtGui/QGraphicsPixmapItem>

namespace WorldMap
{

    class ImageItem : public QObject, public QGraphicsPixmapItem
    {
        Q_OBJECT

        public:
            ImageItem(const QPixmap &pixmap, QGraphicsItem *parent = 0,QGraphicsScene *scene = 0);
            virtual ~ImageItem();
            void SetAvatarName(QString avatar_name) { avatar_name_ = avatar_name; }
            void SetMyAvatar(bool my_avatar);
            void SetTextPosition(qreal position_x, qreal position_y) { position_x_ = position_x; position_y_ = position_y; }
            void UpdateTextPosition();

        protected:
            void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
            void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);

        private:
            QString avatar_name_;
            QGraphicsScene *scene_;
            QGraphicsTextItem *text_;
            qreal position_x_;
            qreal position_y_;
            QPixmap default_image_;
            QPixmap hover_image_;
            QPixmap my_avatar_image_;
            bool my_avatar_;

    };
}
#endif
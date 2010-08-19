// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "ImageItem.h"

#include "MemoryLeakCheck.h"

namespace WorldMap
{
    ImageItem::ImageItem(const QPixmap &pixmap, QGraphicsItem *parent,
                         QGraphicsScene *scene)                        
                         : QGraphicsPixmapItem(pixmap, parent, scene), 
                         hover_image_("./data/ui/images/worldmap/avatar_hover.png"),
                         my_avatar_image_("./data/ui/images/worldmap/my_avatar.png"),
                         default_image_("./data/ui/images/worldmap/default_avatar.png")                         
    {
        scene_ = scene;
        my_avatar_ = false;
        text_ = new QGraphicsTextItem(parent, scene_);
        default_image_ = pixmap;
        setAcceptsHoverEvents(true);
    }

    ImageItem::~ImageItem()
    {
    }

    void ImageItem::hoverEnterEvent(QGraphicsSceneHoverEvent *)
    {
        text_->setHtml(QString(avatar_name_));
        text_->setPos(position_x_ + 11, position_y_ - 10);
        text_->show();
        setPixmap(hover_image_); 
    }

    void ImageItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *)
    {
        text_->hide();
        if (my_avatar_)
            setPixmap(my_avatar_image_);
        else
            setPixmap(default_image_);
    }

    void ImageItem::UpdateTextPosition()
    {
        if (isVisible())
            text_->setPos(position_x_ + 11, position_y_ - 10);
    }

    void ImageItem::SetMyAvatar(bool my_avatar)
    {
        my_avatar_ = my_avatar;        
        setPixmap(my_avatar_image_);
    }
}

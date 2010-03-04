// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "InfoCard.h"

#include <QRadialGradient>
#include <QGraphicsDropShadowEffect>
#include <QGraphicsSceneMouseEvent>
#include <QFile>

#include <QDebug>

#include "MemoryLeakCheck.h"

namespace Ether
{
    namespace View
    {
        InfoCard::InfoCard(ArragementType type, QRectF bounding_rect, QUuid mapping_id, QString title, QString pixmap_path)
            : QGraphicsWidget(),
              type_(type),
              id_(mapping_id),
              title_(title),
              pixmap_path_(pixmap_path),
              pixmap_(QPixmap()),
              bounding_rectf_(bounding_rect),
              active_animations_(),
              frame_pixmap_(bounding_rect.size().toSize()),
              combined_pixmap_(bounding_rect.size().toSize())
        {
            resize(bounding_rectf_.size());

            if (type == TopToBottom)
            {
                data_type_ = Avatar;
                frame_pixmap_.load("./data/ui/images/ether/card_frame_top.png", "PNG");
            }
            else if (type == BottomToTop)
            {
                data_type_ = World;
                frame_pixmap_.load("./data/ui/images/ether/card_frame_bottom.png", "PNG");
            }
            
            UpdatePixmap(pixmap_path);
            InitPaintHelpers();
            InitDecorations();
        }

        void InfoCard::UpdatePixmap(QString pixmap_path)
        {
            // Check that file is valid, if not use default images
            if (!QFile::exists(pixmap_path))
            {
                if (dataType() == Avatar)
                    pixmap_path_ = "./data/ui/images/ether/naali.png";
                else if (dataType() == World)
                    pixmap_path_ = "./data/ui/images/ether/world.png";
            }
            else
                pixmap_path_ = pixmap_path;

            // Setup pixmap
            QSize image_size(bounding_rectf_.width()-4*2, bounding_rectf_.height()-18-11);
            pixmap_.load(pixmap_path_);

            if (pixmap_.rect().width() < image_size.width() && pixmap_.rect().height() < image_size.height())
                pixmap_ = pixmap_.scaled(image_size, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
            else
            {
                pixmap_ = pixmap_.scaledToHeight(image_size.height(), Qt::SmoothTransformation);
                pixmap_ = pixmap_.copy(QRect(QPoint(pixmap_.width()/2-image_size.width()/2,0), QPoint(pixmap_.width()/2+image_size.width()/2, pixmap_.height())));
            }

            combined_pixmap_.fill(Qt::transparent);
            QPainter p(&combined_pixmap_);
            p.drawPixmap(QPointF(0,0), frame_pixmap_, bounding_rectf_);
            if (type_ == TopToBottom)
                p.drawPixmap(QPointF(4,18), pixmap_);
            else if (type_ == BottomToTop)
                p.drawPixmap(QPointF(4,11), pixmap_);
        }

        void InfoCard::InitPaintHelpers()
        {          
            // Font and pen
            font_ = QFont("Helvetica", 10);
            pen_ = QPen(Qt::SolidLine);
            pen_.setColor(Qt::blue);

            // Background brush
            QRadialGradient bg_grad(QPointF(80, 80), 100);
            bg_grad.setColorAt(0, QColor(255,255,255,255));
            bg_grad.setColorAt(1, QColor(90,110,200));
            bg_brush_ = QBrush(bg_grad);
        }

        void InfoCard::InitDecorations()
        {
            QGraphicsDropShadowEffect *effect = new QGraphicsDropShadowEffect(this);
            effect->setColor(QColor(0,0,0,255));
            effect->setOffset(0,5);
            effect->setBlurRadius(15);

            active_animations_ = new QSequentialAnimationGroup(this);
            blur_animation_ = new QPropertyAnimation(effect, "blurRadius", active_animations_);
            blur_animation_->setEasingCurve(QEasingCurve::Linear);
            blur_animation_->setDuration(750);
            blur_animation_->setStartValue(30);
            blur_animation_->setKeyValueAt(0.5, 5);
            blur_animation_->setEndValue(30);
            blur_animation_->setLoopCount(-1);

            jump_animation_ = new QPropertyAnimation(effect, "blurRadius", active_animations_);
            jump_animation_->setEasingCurve(QEasingCurve::Linear);
            jump_animation_->setDuration(400);
            jump_animation_->setStartValue(0);
            jump_animation_->setKeyValueAt(0.5, 50);
            jump_animation_->setEndValue(25);

            active_animations_->addAnimation(jump_animation_);
            active_animations_->addAnimation(blur_animation_);

            setGraphicsEffect(effect);
            setTransformOriginPoint(QPointF(bounding_rectf_.width()/2, bounding_rectf_.top()));
        }

        void InfoCard::IsActiveItem(bool active)
        {
            QGraphicsDropShadowEffect *effect = dynamic_cast<QGraphicsDropShadowEffect *>(graphicsEffect());
            if (active)
            {          
                if (effect)
                    effect->setColor(Qt::black);
                active_animations_->start();
            }
            else
            {
                if (effect)
                    effect->setColor(Qt::black);
                active_animations_->stop();
                effect->setBlurRadius(20);
            }
        }

        QRectF InfoCard::boundingRect() const
        {
            return bounding_rectf_;
        }

        void InfoCard::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
        {
            painter->drawPixmap(QPointF(0,0), combined_pixmap_, bounding_rectf_);
        }
    }
}

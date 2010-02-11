// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "EllipseMenu.h"

#include <QPointF>
#include <math.h>
#define M_PI_2     1.57079632679489661923 // In naali this didnt get defined from math.h

#include <QPointF>
#include <QPropertyAnimation>
#include <QDebug>

namespace Ether
{
    namespace View
    {
        EllipseMenu::EllipseMenu(TYPE type)
            : QObject(),
              highlighted_(0),
              animations_(new QParallelAnimationGroup()),
              max_visible_objects_(0),
              left_visible_limit_index_(0),
              right_visible_limit_index_(0),
              opacity_factor_(1),
              scale_factor_(1),
              current_scale_factor_(scale_factor_),
              visible_(1),
              invisible_(0),
              type_(type)
        {

        }

        void EllipseMenu::Initialize(const QRectF& bounds, const QVector<InfoCard *>& items, const QRectF& max_size, qreal scalef, int visible_objects, qreal opacity_factor)
        {
            objects_.clear();
            positions_.clear();
            animations_->clear();
            priority_list_.clear();

            objects_ = items;
            boundaries_ = bounds;
            max_visible_objects_ = visible_objects;
            priority_list_.fill(0, objects_.size());
            current_card_max_size_ = card_max_size_ = max_size;
            opacity_factor_ = opacity_factor;
            current_scale_factor_ = scale_factor_ = scalef;

            CalculatePositions(positions_);

            ConstructPriorityList();
            CalculateLimits();
            InitializeAnimations();
            InitializeObjectValues();
        }

        EllipseMenu::~EllipseMenu()
        {

        }

        void EllipseMenu::SetFocusToCard(InfoCard *card)
        {
            int start_pos = priority_list_.count() % 2 == 1 ? priority_list_.count() / 2 : priority_list_.count() / 2 - 1;
            int position = priority_list_.indexOf(objects_.indexOf(card));
            int move_count = start_pos - position - left_visible_limit_index_;

            if (move_count == 0)
                return;
            else if (move_count > 0)
                for (;move_count > 0; move_count--)
                    moveRight();
            else if (move_count < 0)
                for (;move_count < 0; move_count++)
                    moveLeft();
        }

        void EllipseMenu::InitializeObjectValues()
        {
            QPointF startpos = hide_point_;
            for (int i=0; i<priority_list_.size(); i++)
            {
                InfoCard* obj = objects_.at(priority_list_.at(i));
                if (left_visible_limit_index_ <= i && i <= right_visible_limit_index_)
                {
                    obj->setOpacity(visible_);
                }
                else
                {
                    obj->setPos(hide_point_);
                    obj->setOpacity(invisible_);
                }
            }

            InitializeObjectsToStates();
        }

        void EllipseMenu::CalculateLimits()
        {
            if(objects_.size()==0)
                return;

            int index1 = objects_.size()%2==0?(objects_.size()/2)-1:(objects_.size()/2);
            int index2 = index1;
            int iterator = 1;

            while(iterator<max_visible_objects_ && iterator<objects_.size())
            {
                if(iterator%2==1)
                {
                    index1++;
                    iterator++;
                }
                else
                {
                    index2--;
                    iterator++;
                }
            }
            left_visible_limit_index_=index2;
            right_visible_limit_index_=index1;
        }

        void EllipseMenu::ConstructPriorityList()
        {
            if(objects_.size()==0)
                return;

            int index1 = objects_.size() % 2 == 0 ? (objects_.size()/2)-1 : (objects_.size()/2);
            int index2 = index1;
            int priority = 0;

            priority_list_.replace(index1, 0);
            index1++;
            index2--;
            priority++;

            for(int i=1; i<objects_.size(); i++)
            {

                if(i%2==1)
                {
                    priority_list_.replace(index1, priority);
                    priority++, index1++;
                }
                else
                {
                    priority_list_.replace(index2, priority);
                    priority++, index2--;
                }
            }
        }

        void EllipseMenu::InitializeAnimations()
        {
            for(int i=0; i<objects_.size();i++)
            {
                QParallelAnimationGroup* animgroup = new QParallelAnimationGroup();
                QPropertyAnimation* anim1 = new QPropertyAnimation(objects_.at(i), "pos");
                anim1->setDuration(300);
                anim1->setEasingCurve(QEasingCurve::InOutSine);
                QPropertyAnimation* anim2 = new QPropertyAnimation(objects_.at(i), "scale");
                anim2->setDuration(300);
                anim2->setEasingCurve(QEasingCurve::InOutSine);
                QPropertyAnimation* anim3 = new QPropertyAnimation(objects_.at(i), "opacity");
                anim3->setDuration(300);
                anim3->setEasingCurve(QEasingCurve::InQuad);
                QPropertyAnimation* anim4 = new QPropertyAnimation(objects_.at(i), "z");
                anim3->setDuration(300);
                anim3->setEasingCurve(QEasingCurve::Linear);

                animgroup->addAnimation(anim1);
                animgroup->addAnimation(anim2);
                animgroup->addAnimation(anim3);
                animgroup->addAnimation(anim4);
                animations_->addAnimation(animgroup);

                objects_.at(i)->SetMoveAnimationPointer(anim1);
            }
        }

        void EllipseMenu::ShowCardAtIndex(int index)
        {
            InfoCard* obj = objects_.at(index);
            QPropertyAnimation* animopa = dynamic_cast<QPropertyAnimation*>(dynamic_cast<QParallelAnimationGroup*>(animations_->animationAt(index))->animationAt(2));
            if(animopa)
            {
                animopa->stop();
                animopa->setStartValue(obj->opacity());
                animopa->setEndValue(visible_);
                animopa->start();
            }
        }

        void EllipseMenu::HideCardAtIndex(int index)
        {
            InfoCard* obj = objects_.at(index);
            //all visible objects have higher Z order
            ZOrderObjectBasedOnPriority(index, max_visible_objects_ + 1, true);

            QPropertyAnimation* animpos = dynamic_cast<QPropertyAnimation*>(dynamic_cast<QParallelAnimationGroup*>(animations_->animationAt(index))->animationAt(0));
            if(animpos)
            {
                animpos->stop();
                animpos->setStartValue(obj->pos());
                animpos->setEndValue(hide_point_);
                animpos->start();
            }

            QPropertyAnimation* animopa = dynamic_cast<QPropertyAnimation*>(dynamic_cast<QParallelAnimationGroup*>(animations_->animationAt(index))->animationAt(2));
            if(animopa)
            {
                animopa->stop();
                animopa->setStartValue(obj->opacity());
                animopa->setEndValue(invisible_);
                animopa->start();
            }
        }


        void EllipseMenu::AssignObjectsToStates()
        {
            int object_index;
            int priority=0;
            int index2 = objects_.size()%2==0?(objects_.size()/2)-1:(objects_.size()/2);
            int index1 = index2+1;
            int iterator = 0;

            while (iterator < max_visible_objects_ && iterator < objects_.size())
            {
                if (iterator % 2 == 1)
                {
                    object_index = priority_list_.at(index1);
                    index1++;
                    iterator++;
                }
                else
                {
                    object_index = priority_list_.at(index2);
                    index2--;
                    iterator++;
                }

                // Move animation with object index and end pos
                MoveObjectToPosition(object_index, positions_.at(priority));
                // Scale animation
                ScaleObjectBasedOnPriority(object_index,priority);
                // Z order animation
                ZOrderObjectBasedOnPriority(object_index,priority, true);

                ChangeObjectOpacityBasedOnPriority(object_index,priority);

                if (priority == 0 && highlighted_ != objects_.at(object_index))
                {
                    highlighted_ = objects_.at(object_index);
                    emit ItemHighlighted(highlighted_);
                }
                priority++;
            }
        }


        void EllipseMenu::InitializeObjectsToStates()
        {
            int object_index;
            int priority = 0;
            int index2 = objects_.size() % 2 == 0 ?(objects_.size()/2)-1:(objects_.size()/2);
            int index1 = index2 + 1;
            int iterator = 0;

            while (iterator < max_visible_objects_ && iterator < objects_.size())
            {
                if (iterator % 2 == 1)
                {
                    object_index = priority_list_.at(index1);
                    index1++;
                    iterator++;
                }
                else
                {
                    object_index = priority_list_.at(index2);
                    index2--;
                    iterator++;
                }

                // Move animation with object index and end pos
                MoveObjectToPosition(object_index, positions_.at(priority));
                // Scale animation
                ScaleObjectBasedOnPriority(object_index,priority);
                // Z order animation
                ZOrderObjectBasedOnPriority(object_index,priority, false);

                ChangeObjectOpacityBasedOnPriority(object_index,priority);

                if (priority == 0 && highlighted_ != objects_.at(object_index))
                {
                    highlighted_ = objects_.at(object_index);
                    emit ItemHighlighted(highlighted_);
                }
                priority++;
            }
        }

        InfoCard *EllipseMenu::GetHighlighted()
        {
            int index2 = priority_list_.size() % 2 == 0 ?(priority_list_.size()/2)-1:(priority_list_.size()/2);
            return objects_.at(priority_list_.at(index2));
        }
       

        void EllipseMenu::ZOrderObjectBasedOnPriority(int index, int priority, bool animated)
        {
            InfoCard* obj = objects_.at(index);
            if (animated)
            {
                QPropertyAnimation* anim = dynamic_cast<QPropertyAnimation*>(dynamic_cast<QParallelAnimationGroup*>(animations_->animationAt(index))->animationAt(3));
                if (anim)
                {
                    anim->stop();
                    anim->setStartValue(obj->scale());
                    anim->setEndValue(max_visible_objects_- priority);
                    anim->start();
                }
            }
            else
            {
                obj->setZValue(max_visible_objects_- priority);
            }
        }

        void EllipseMenu::ChangeObjectOpacityBasedOnPriority(int index,int priority)
        {
            if(opacity_factor_>0)
            {
                InfoCard* obj = objects_.at(index);
                int opacity_level = (priority+1)/2;
                QPropertyAnimation* anim = dynamic_cast<QPropertyAnimation*>(dynamic_cast<QParallelAnimationGroup*>(animations_->animationAt(index))->animationAt(2));
                if (anim)
                {
                    anim->stop();
                    anim->setStartValue(obj->opacity());
                    anim->setEndValue(1- opacity_level*opacity_factor_);
                    anim->start();
                }
            }
        }

        void EllipseMenu::ScaleObjectBasedOnPriority(int index, int priority)
        {
            qreal scalevalue;
            InfoCard* obj = objects_.at(index);


            if(priority==0 && card_max_size_.height() > boundaries_.height())
            {
                scalevalue =  current_card_max_size_.height() / (card_max_size_.height());

                //A hacky adjust so that bottom of the bottom card doesn't go below the scene border
                scalevalue *= 0.95;

            }
            else
            {
                qreal scalemultiplier = priority%2==0?(priority/2):(priority/2)+1;
                scalevalue = pow(current_scale_factor_, scalemultiplier*0.8);
                scalevalue *= 0.9;
            }


            QPropertyAnimation* anim = dynamic_cast<QPropertyAnimation*>(dynamic_cast<QParallelAnimationGroup*>(animations_->animationAt(index))->animationAt(1));
            if (anim)
            {
                anim->stop();
                anim->setStartValue(obj->scale());
                anim->setEndValue(scalevalue);
                anim->start();
            }
        }

        void EllipseMenu::moveLeft()
        {
            if (objects_.size() <= 1)
                return;

            int front = priority_list_.first();
            priority_list_.remove(0);
            priority_list_.push_back(front);

            if (objects_.size() > max_visible_objects_)
            {
                ShowCardAtIndex(priority_list_.at(right_visible_limit_index_));
                HideCardAtIndex(priority_list_.at(  (left_visible_limit_index_-1>=0?left_visible_limit_index_-1:priority_list_.size()-1)  ));
            }

            AssignObjectsToStates();
        }

        void EllipseMenu::moveRight()
        {
            if (objects_.size() <= 1)
                return;

            int back = priority_list_.last();
            priority_list_.remove(priority_list_.size()-1);
            priority_list_.prepend(back);

            if (objects_.size() > max_visible_objects_)
            {
                ShowCardAtIndex(priority_list_.at(left_visible_limit_index_));
                HideCardAtIndex(priority_list_.at((right_visible_limit_index_+1)%priority_list_.size()));
            }

            AssignObjectsToStates();
        }

        void EllipseMenu::MoveObjectToPosition(int index,const QPointF& point)
        {
            InfoCard* obj = objects_.at(index);
            QPropertyAnimation* anim = dynamic_cast<QPropertyAnimation*>(dynamic_cast<QParallelAnimationGroup*>(animations_->animationAt(index))->animationAt(0));
            if(anim)
            {
                anim->setStartValue(obj->pos());
                anim->setEndValue(point);
                anim->start();
            }
        }

        void EllipseMenu::RectChanged(const QRectF& bounds)
        {
            boundaries_ = bounds;
            positions_.clear();

            if(bounds.height()< card_max_size_.height())
            {
                current_scale_factor_ = scale_factor_ - (1- (bounds.height()/ card_max_size_.height()));
                current_scale_factor_ = current_scale_factor_;
                current_card_max_size_.setHeight( bounds.height() );

            }
            else
            {
                current_scale_factor_ = scale_factor_;
                current_card_max_size_.setHeight( card_max_size_.height() );
            }

            CalculatePositions(positions_);
            InitializeObjectValues();
        }

        void EllipseMenu::CalculatePositions(QVector<QPointF> &positions)
        {
            QRectF boundaries = boundaries_;

            qreal start_phase;

            qreal end_phase;

            int visible_objects = max_visible_objects_;

            boundaries.setWidth(boundaries.width() - current_card_max_size_.width());

            qreal width_radius = boundaries.width()/2;

            qreal phase_change = 0;
            qreal height_bias = 0;

            if(type_ == EllipseMenu::OPENS_UP)
            {
                start_phase = M_PI_2;
                end_phase = 0;
                phase_change = -fabs(end_phase - start_phase) / ((visible_objects/2));
                height_bias = boundaries.top();
                boundaries.setBottom(boundaries.bottom()-current_card_max_size_.height());
            }
            if(type_ == EllipseMenu::OPENS_DOWN)
            {

                start_phase = -M_PI_2;
                end_phase = 0;
                phase_change = fabs(end_phase - start_phase) / ((visible_objects/2));
                boundaries.setBottom(boundaries.bottom()-current_card_max_size_.height());
                height_bias = boundaries.bottom();
            }

            qreal current_phase = start_phase;

            QPointF pos;

            pos.setX(boundaries.left()+width_radius);
            pos.setY(sin(current_phase)*boundaries.height()+ height_bias);

            positions.push_back(pos);

            //calculate the rest of the positions
            for (int i=1; i<visible_objects; i++)
            {
                QPointF pos;
                qreal cosine=0;

                //if indexer is paired, we will negate the cos of phase
                if(i%2==1)
                {
                    current_phase += phase_change;
                    cosine = cos(current_phase);
                }
                else
                {
                    cosine = -cos(current_phase);
                }
                pos.setX(cosine*width_radius+boundaries.left()+ width_radius);
                pos.setY(sin(current_phase)*boundaries.height()+height_bias);
                positions.push_back(pos);
            }

            hide_point_.setX(boundaries.right()/2);
            if(type_ == OPENS_UP)
                hide_point_.setY(boundaries.top());
            if(type_ == OPENS_DOWN)
                hide_point_.setY(boundaries.bottom());
        }
    }
}

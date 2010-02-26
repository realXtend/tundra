// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "VerticalMenu.h"

#include <QDebug>
#include <math.h>

#include "MemoryLeakCheck.h"

namespace Ether
{
    namespace View
    {
        VerticalMenu::VerticalMenu(TYPE type)
        {
            type_ = type;
        }

        VerticalMenu::~VerticalMenu()
        {

        }
        void VerticalMenu::CalculatePositions(QVector<QPointF> &positions)
        {
            QRectF boundaries = boundaries_;
            
            //this is needed when rect size is smaller than the cards max size because we need to shrink the highlighted card too
            qreal additional_shrinkage = 0;

            if(current_scale_factor_ != scale_factor_)
            {
                
                additional_shrinkage = card_max_size_.width() - (card_max_size_.width()*(current_card_max_size_.height() / (card_max_size_.height())));
                additional_shrinkage/=2;
            }


            int visible_objects = max_visible_objects_;

            boundaries.setWidth(boundaries.width() - current_card_max_size_.width());
           

            qreal width_radius = boundaries.width()/2;

            qreal last_left_pos = width_radius, last_right_pos = width_radius;

            qreal y_coord = 0;

            if(type_ == VerticalMenu::VERTICAL_TOP)
            {
                y_coord = boundaries.top();
            }
            else if(type_ == VerticalMenu::VERTICAL_MID)
            {
                y_coord = boundaries.top() + ((boundaries.height()/2) - (current_card_max_size_.height()/2));
            }

            else if(type_ == VerticalMenu::VERTICAL_BOTTOM)
            {
                y_coord = boundaries.bottom() - (current_card_max_size_.height());
            }

            QPointF pos;

            pos.setX(boundaries.left()+width_radius);
            pos.setY(y_coord);

            positions.push_back(pos);


            //calculate the rest of the positions
            for (int i=1; i<visible_objects; i++)
            {
                qreal current_vert_pos;
                QPointF pos;

                qreal shrinkage = card_max_size_.width() - (card_max_size_.width() * pow(current_scale_factor_ ,(i+1)/2));
                shrinkage/=2;


                if(i%2==1)
                {
                    //Vertical Pos
                    if(type_ == VerticalMenu::VERTICAL_BOTTOM)
                    {
                        if(i==1 && (scale_factor_!= current_scale_factor_))
                        {
                            y_coord +=  (current_card_max_size_.height() - (card_max_size_.height() * current_scale_factor_));
                        }
                        else
                        {
                            y_coord +=  card_max_size_.height()*pow(current_scale_factor_ ,((i+1)/2)-1) - (card_max_size_.height() * pow(current_scale_factor_ ,(i+1)/2));
                        }
                    }
                    else if(type_ == VerticalMenu::VERTICAL_MID)
                    {
                        if(i==1 && (scale_factor_!= current_scale_factor_))
                        {
                            y_coord +=  (current_card_max_size_.height() - (card_max_size_.height() * current_scale_factor_))/2;
                        }
                        else
                        {
                            y_coord +=  (card_max_size_.height()*pow(current_scale_factor_ ,((i+1)/2)-1) - (card_max_size_.height() * pow(current_scale_factor_ ,(i+1)/2)))/2;
                        }
                    }

                    //Horizontal Pos
                    current_vert_pos = last_right_pos + current_card_max_size_.width() -shrinkage + current_gap_;
                    if(i==1)
                    {
                        current_vert_pos -= additional_shrinkage;
                    }

                    last_right_pos = current_vert_pos - shrinkage;
                }
                else
                {
                    current_vert_pos = last_left_pos - current_card_max_size_.width() + shrinkage - current_gap_;
                    if(i==2)
                    {
                        current_vert_pos += additional_shrinkage;
                    }
                    last_left_pos = current_vert_pos + shrinkage;
                }
                pos.setX(current_vert_pos+boundaries.left());
                pos.setY(y_coord);
                positions.push_back(pos);
            }
            
            hide_point_.setX(positions.first().x());
            hide_point_.setY(positions.first().y());
        }
    }
}
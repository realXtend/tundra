// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "EllipseMenu.h"

#include "MemoryLeakCheck.h"

#include <math.h>
#define M_PI_2     1.57079632679489661923 // In naali this didnt get defined from math.h

namespace Ether
{
    namespace View
    {
        EllipseMenu::EllipseMenu(TYPE type)
        {
            type_ = type;
        }

        EllipseMenu::~EllipseMenu()
        {

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

            if(type_ == EllipseMenu::ELLIPSE_OPENS_UP)
            {
                start_phase = M_PI_2;
                end_phase = 0;
                phase_change = -fabs(end_phase - start_phase) / ((visible_objects/2));
                height_bias = boundaries.top();
                boundaries.setBottom(boundaries.bottom()-current_card_max_size_.height());
            }
            if(type_ == EllipseMenu::ELLIPSE_OPENS_DOWN)
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




                //since sometimes rect can be so small, that the points are forming a line, we want to adjust the cards a bit
                /*if(type_ == EllipseMenu::OPENS_DOWN)
                {
                    qreal scaled_h = current_card_max_size_.height() * pow(current_scale_factor_ ,static_cast<qreal>((i+1)/2));
                    qreal bot = pos.y() + scaled_h;
                    if(bot<  (boundaries.top() + current_card_max_size_.height()))
                    {
                        pos.setY(boundaries.top() + (current_card_max_size_.height() - scaled_h));
                    }
                }*/

                positions.push_back(pos);
            }

            hide_point_.setX(boundaries.right()/2);
            if(type_ == EllipseMenu::ELLIPSE_OPENS_UP)
                hide_point_.setY(boundaries.top());
            if(type_ == EllipseMenu::ELLIPSE_OPENS_DOWN)
                hide_point_.setY(boundaries.bottom());
        }
    }
}

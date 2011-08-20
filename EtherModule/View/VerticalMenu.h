// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_EtherModule_VerticalMenu_h
#define incl_EtherModule_VerticalMenu_h

#include "EtherMenu.h"

namespace Ether
{
    namespace View
    {
        //! Class for animated ellipse menu.
        //! note that this class is NOT responsible of deleting the QGraphicsobjects it animates
        class VerticalMenu : public EtherMenu
        {
        public:

            enum TYPE {VERTICAL_TOP, VERTICAL_MID, VERTICAL_BOTTOM };

            VerticalMenu(TYPE type);
            virtual ~VerticalMenu();
        private:
            //! Calculate positions for objects
            void CalculatePositions(QVector<QPointF> &positions);


            TYPE type_;
        };
    }
}

#endif // VerticalMenu_H

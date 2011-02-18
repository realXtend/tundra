// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_EllipseMenu_h
#define incl_UiModule_EllipseMenu_h

#include "EtherMenu.h"

namespace Ether
{
    namespace View
    {
        //! Class for animated ellipse menu.
        //! note that this class is NOT responsible of deleting the QGraphicsobjects it animates
        class EllipseMenu : public EtherMenu
        {
        public:

            enum TYPE { ELLIPSE_OPENS_UP, ELLIPSE_OPENS_DOWN};

            EllipseMenu(TYPE type);
            virtual ~EllipseMenu();
        private:
            //! Calculate positions for objects
            void CalculatePositions(QVector<QPointF> &positions);

            TYPE type_;
        };
    }
}

#endif // ELLIPSEMENU_H

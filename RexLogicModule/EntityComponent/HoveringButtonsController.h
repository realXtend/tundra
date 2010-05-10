/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
*/
#ifndef incl_HoveringButtonsController_h
#define incl_HoveringButtonsController_h

#include "ui_HoveringButtonsWidget.h"
#include <QWidget>
#include <CoreTypes.h>
#include <QRect>

class QPushButton;

namespace RexLogic
{
    class HoveringButtonsController: public QWidget, private Ui::HoveringButtonsWidget
    {
        Q_OBJECT

    public:
        HoveringButtonsController();
        virtual ~HoveringButtonsController();

        void AddButton(QPushButton *button);
        void ForwardMouseClickEvent(Real x, Real y);

    public slots:
        void ButtonPressed();

    private:
        Real text_padding_;


    };
}

#endif
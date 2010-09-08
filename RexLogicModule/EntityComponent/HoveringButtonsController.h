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
    //class controlling the hovering buttons
    class HoveringButtonsController: public QWidget, private Ui::HoveringButtonsWidget
    {
        Q_OBJECT

    public:
        //Ctor
        HoveringButtonsController();
        //Dtor
        virtual ~HoveringButtonsController();

        //Adds button to this widget
        void AddButton(QPushButton *button);

        //notifies the widget that is clicked (if any)
        void ForwardMouseClickEvent(float x, float y);



    public slots:

        void ButtonPressed();

    private:
        float text_padding_;


    };
}

#endif
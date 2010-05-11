/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
*/
#ifndef incl_HoveringNameController_h
#define incl_HoveringNameController_h

#include "ui_HoveringNameWidget.h"
#include <QWidget>
#include <CoreTypes.h>
#include <QRect>

namespace RexLogic
{
    class HoveringNameController: public QWidget, private Ui::HoveringName
    {
        Q_OBJECT

    public:
        HoveringNameController();
        virtual ~HoveringNameController();

        void SetText(const QString& text);

    private:
        Real text_padding_;


    };
}

#endif
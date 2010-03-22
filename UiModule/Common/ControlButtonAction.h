// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_ControlButtonAction_h
#define incl_UiModule_ControlButtonAction_h

#include "UiAction.h"
#include "Inworld/ControlPanel/ControlPanelButton.h"

namespace CoreUi
{
    class ControlButtonAction : public UiServices::UiAction
    {

    Q_OBJECT

    public:
        ControlButtonAction(ControlPanelButton *control_button, QGraphicsWidget *controlled_widget, QObject *parent = 0);

    public slots:
        void RequestHide();

    private:
        QGraphicsWidget *controlled_widget_;
    };
}

#endif
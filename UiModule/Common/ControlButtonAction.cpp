// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "ControlButtonAction.h"

namespace CoreUi
{
    ControlButtonAction::ControlButtonAction(ControlPanelButton *control_button, QGraphicsWidget *controlled_widget, QObject *parent) :
        UiServices::UiAction(parent, true),
        controlled_widget_(controlled_widget)
    {
        connect(this, SIGNAL(toggled(bool)), control_button, SLOT(CheckStyle(bool)));
    }

    void ControlButtonAction::RequestHide()
    {
        if (controlled_widget_->isVisible())
            trigger();
    }
}
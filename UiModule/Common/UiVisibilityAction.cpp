// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "UiVisibilityAction.h"

namespace UiServices
{
    UiVisibilityAction::UiVisibilityAction(QGraphicsWidget *controlled_widget, QObject *parent) :
        UiAction(parent, true),
        controlled_widget_(controlled_widget)
    {
        connect(this, SIGNAL(toggled(bool)), SLOT(ToggleVisibility()));
    }

    void UiVisibilityAction::SetControlledWidget(QGraphicsWidget *controlled_widget)
    {
        controlled_widget_ = controlled_widget;
    }

    void UiVisibilityAction::ToggleVisibility()
    {
        if (!controlled_widget_)
            return;

        if (controlled_widget_->isVisible())
            controlled_widget_->hide();
        else
            controlled_widget_->show();

        emit VisibilityChanged(controlled_widget_->isVisible());
    }
}
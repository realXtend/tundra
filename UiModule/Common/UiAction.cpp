// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "UiAction.h"

namespace UiServices
{
    UiAction::UiAction(QObject *parent, bool checkable) :
        QAction(parent)
    {
        if (checkable)
        {
            setCheckable(true);
            setChecked(false);
        }
    }

    void UiAction::trigger()
    {
        if (isCheckable())
            toggle();
        else
            QAction::trigger();
    }
}
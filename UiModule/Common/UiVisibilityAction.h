// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_UiVisibilityAction_h
#define incl_UiModule_UiVisibilityAction_h

#include "UiModuleApi.h"
#include "UiAction.h"

#include <QGraphicsWidget>

namespace UiServices
{
    class UI_MODULE_API UiVisibilityAction : public UiAction
    {

    Q_OBJECT

    public:
        UiVisibilityAction(QGraphicsWidget *controlled_widget, QObject *parent = 0);

    public slots:
        void SetControlledWidget(QGraphicsWidget *controlled_widget);
        void ToggleVisibility();

    signals:
        void VisibilityChanged(bool visible);

    private:
        QGraphicsWidget *controlled_widget_;

    };
}

#endif
// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_UiAction_h
#define incl_UiModule_UiAction_h

#include "UiModuleApi.h"

#include <QGraphicsWidget>
#include <QAction>

namespace UiServices
{
    class UI_MODULE_API UiAction : public QAction
    {

    Q_OBJECT

    public:
        UiAction(QObject *parent = 0, bool checkable = false);
        
    public slots:
        void trigger();

    };
}

#endif
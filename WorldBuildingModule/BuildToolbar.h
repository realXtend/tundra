// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_WorldBuildingModule_BuildToolbar_h
#define incl_WorldBuildingModule_BuildToolbar_h

#include <QGraphicsProxyWidget>
#include "ui_Toolbar.h"

namespace WorldBuilding
{
    namespace Ui
    {
        class BuildToolbar : public QGraphicsProxyWidget, public ::Ui::Toolbar
        {
        
        Q_OBJECT

        public:
            BuildToolbar();

        public slots:
            void AddButton(const QString &name, QGraphicsProxyWidget *widget);
            void RemoveAllButtons();

        private:
            QWidget *widget_;
        };
    }
}

#endif
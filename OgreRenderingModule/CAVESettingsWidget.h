// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OgreRenderingModule_CaveSettingsWidget_h
#define incl_OgreRenderingModule_CaveSettingsWidget_h

#include <QWidget>
#include "ui_CaveSettings.h"

namespace Foundation
{
    class Framework;
}
namespace OgreRenderer
{
    
    class CAVESettingsWidget: public QWidget, private Ui::CAVESettings 
    {
        Q_OBJECT
    public:
        CAVESettingsWidget( Foundation::Framework* framework, QWidget* parent=0 );

    signals:
        void ToggleCAVE(bool);

    public slots:
        void CAVEButtonToggled(bool v);

    private:
        Foundation::Framework* framework_;

    };

}
#endif
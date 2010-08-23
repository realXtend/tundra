// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OgreRenderingModule_CaveSettingsWidget_h
#define incl_OgreRenderingModule_CaveSettingsWidget_h

#include <QWidget>
#include "ui_CaveSettings.h"
#include "CAVEViewSettings.h"
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
        void NewCAVEViewRequested(const QString& name, Ogre::Vector3 &tl, Ogre::Vector3 &bl, Ogre::Vector3 &br, Ogre::Vector3 &eye);

    public slots:
        void CAVEButtonToggled(bool v);
        void AddNewCAVEView();

    private:
        
        Foundation::Framework* framework_;
        CAVEViewSettings settings_dialog_;
        int next_view_num_;
        QString view_prefix_;

    };

}
#endif
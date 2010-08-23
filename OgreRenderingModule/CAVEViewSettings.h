// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OgreRenderingModule_CaveViewSettings_h
#define incl_OgreRenderingModule_CaveViewSettings_h

#include "ui_CaveViewSettingsDialog.h"
#include <QDialog>

namespace Ogre
{
    class Vector3;
}

namespace OgreRenderer
{

    class CAVEViewSettings: public QDialog, private Ui::CaveViewSettingsDialog
    {
        Q_OBJECT
    public:
        void getCaveProjectionSettings(Ogre::Vector3 &eye_pos, Ogre::Vector3 &bottom_left, Ogre::Vector3 &top_left, Ogre::Vector3 &bottom_right);
        CAVEViewSettings( QWidget* parent=0 );
        ~CAVEViewSettings();


    };

}
#endif
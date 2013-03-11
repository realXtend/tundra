// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "ui_CaveSettingsDialogAdvanced.h"
#include <QDialog>

namespace Ogre
{
    class Vector3;
}

namespace CAVEStereo
{
    class CAVEViewSettingsAdvanced: public QDialog, private Ui::CaveViewSettingsDialogAdvanced
    {
        Q_OBJECT
    public:
        void getCaveProjectionSettings(Ogre::Vector3 &eye_pos, Ogre::Vector3 &bottom_left, Ogre::Vector3 &top_left, Ogre::Vector3 &bottom_right);
        void setCaveProjectionSettings(Ogre::Vector3 &eye_pos, Ogre::Vector3 &bottom_left, Ogre::Vector3 &top_left, Ogre::Vector3 &bottom_right);
        CAVEViewSettingsAdvanced( QWidget* parent=0 );
        ~CAVEViewSettingsAdvanced();
    };
}

// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "ui_CaveViewSettingsDialog.h"
#include <QDialog>

namespace Ogre
{
    class Vector3;
}

namespace CAVEStereo
{
    class CAVEViewSettings: public QDialog, private Ui::CaveViewSettingsDialog
    {
        Q_OBJECT
    public:
        void getCaveProjectionSettings(Ogre::Vector3 &eye_pos, Ogre::Vector3 &bottom_left, Ogre::Vector3 &top_left, Ogre::Vector3 &bottom_right);
        static void ConvertToVectors(qreal hor_angle, qreal ver_angle, qreal dist, qreal fovx, qreal fovy, Ogre::Vector3 &bottom_left, Ogre::Vector3 &top_left, Ogre::Vector3 &bottom_right);
        CAVEViewSettings( QWidget* parent=0 );
        ~CAVEViewSettings();
    };

}

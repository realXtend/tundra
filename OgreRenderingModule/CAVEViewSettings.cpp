#include "StableHeaders.h"
#include "CAVEViewSettings.h"
#include <Ogre.h>

namespace OgreRenderer
{
    CAVEViewSettings::CAVEViewSettings( QWidget* parent )
        :QDialog(parent)
    {
        setupUi(this);
        

    }

    

    CAVEViewSettings::~CAVEViewSettings()
    {

    }

    void CAVEViewSettings::getCaveProjectionSettings(Ogre::Vector3 &eye_pos, Ogre::Vector3 &bottom_left, Ogre::Vector3 &top_left, Ogre::Vector3 &bottom_right)
    {
        eye_pos.x = epxv->value();
        eye_pos.y = epyv->value();
        eye_pos.z = epzv->value();

        bottom_left.x = blxv->value();
        bottom_left.y = blyv->value();
        bottom_left.z = blzv->value();

        top_left.x = tlxv->value();
        top_left.y = tlyv->value();
        top_left.z = tlzv->value();

        bottom_right.x = brxv->value();
        bottom_right.y = bryv->value();
        bottom_right.z = brzv->value();


    }
}
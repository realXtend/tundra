#include "StableHeaders.h"
#include "CAVEViewSettingsAdvanced.h"
namespace OgreRenderer
{
    CAVEViewSettingsAdvanced::CAVEViewSettingsAdvanced( QWidget* parent )
        :QDialog(parent)
    {
        setupUi(this);
        

    }

    

    CAVEViewSettingsAdvanced::~CAVEViewSettingsAdvanced()
    {

    }

    void CAVEViewSettingsAdvanced::getCaveProjectionSettings(Ogre::Vector3 &eye_pos, Ogre::Vector3 &bottom_left, Ogre::Vector3 &top_left, Ogre::Vector3 &bottom_right)
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

    void CAVEViewSettingsAdvanced::setCaveProjectionSettings(Ogre::Vector3 &eye_pos, Ogre::Vector3 &bottom_left, Ogre::Vector3 &top_left, Ogre::Vector3 &bottom_right)
    {
        epxv->setValue(  eye_pos.x );
        epyv->setValue(  eye_pos.y );
        epzv->setValue(  eye_pos.z );

        blxv->setValue(  bottom_left.x );
        blyv->setValue(  bottom_left.y );
        blzv->setValue(  bottom_left.z );

        tlxv->setValue(  top_left.x );
        tlyv->setValue(  top_left.y );
        tlzv->setValue(  top_left.z );

        brxv->setValue(  bottom_right.x );
        bryv->setValue(  bottom_right.y );
        brzv->setValue(  bottom_right.z );
    }
}
#include "StableHeaders.h"
#include "CAVEViewSettings.h"
#include <Ogre.h>
#include <QDebug>

namespace OgreRenderer
{
    CAVEViewSettings::CAVEViewSettings(QWidget* parent) :
        QDialog(parent)
    {
        setupUi(this);
    }

    CAVEViewSettings::~CAVEViewSettings()
    {
    }

    void CAVEViewSettings::getCaveProjectionSettings(Ogre::Vector3 &eye_pos, Ogre::Vector3 &bottom_left, Ogre::Vector3 &top_left, Ogre::Vector3 &bottom_right)
    {
        //get the angles and calculate the corresponding vectors
        qreal hor_angle = h_angle->value();
        qreal ver_angle = v_angle->value();
        qreal dist = distance->value();

        qreal fovx = fov_hor->value();
        qreal fovy = fov_vert->value();

        
		ConvertToVectors(hor_angle, ver_angle,dist,fovx, fovy,bottom_left,top_left, bottom_right);
        /*
        bottom_left = Ogre::Quaternion(Ogre::Radian((fovx*0.5f)*Ogre::Math::PI/180.f),up)*dir;
        bottom_left = Ogre::Quaternion(Ogre::Radian((fovy*0.5f)*Ogre::Math::PI/180.f),-right)*bottom_left;

        top_left = Ogre::Quaternion(Ogre::Radian((fovx*0.5f)*Ogre::Math::PI/180.f),up)*dir;
        top_left = Ogre::Quaternion(Ogre::Radian((fovy*0.5f)*Ogre::Math::PI/180.f),right)*top_left;

        bottom_right= Ogre::Quaternion(Ogre::Radian((fovx*0.5f)*Ogre::Math::PI/180.f),-up)*dir;
        bottom_right = Ogre::Quaternion(Ogre::Radian((fovy*0.5f)*Ogre::Math::PI/180.f),-right)*bottom_right;
        */
        eye_pos.x = epxv->value();
        eye_pos.y = epyv->value();
        eye_pos.z = epzv->value();
    }

	void CAVEViewSettings::ConvertToVectors(qreal hor_angle, qreal ver_angle, qreal dist, qreal fovx, qreal fovy, Ogre::Vector3 &bottom_left, Ogre::Vector3 &top_left, Ogre::Vector3 &bottom_right)
	{
		Ogre::Quaternion rot(Ogre::Radian(hor_angle*Ogre::Math::PI/180.f), Ogre::Vector3::NEGATIVE_UNIT_Y);
        rot = Ogre::Quaternion(Ogre::Radian(ver_angle*Ogre::Math::PI/180.f), Ogre::Vector3::UNIT_X) * rot;

        const Ogre::Vector3 dir = ( Ogre::Vector3::NEGATIVE_UNIT_Z)*dist;
        const Ogre::Vector3 local_x_axis = Ogre::Vector3::UNIT_X;
        const Ogre::Vector3 local_y_axis = Ogre::Vector3::UNIT_Y;

        Ogre::Vector3 left = Ogre::Quaternion(Ogre::Radian((fovx*0.5f)*Ogre::Math::PI/180.f),local_y_axis)*dir;
        left = left-dir;
        left.y = 0;


        Ogre::Vector3 right = Ogre::Quaternion(Ogre::Radian((fovx*0.5f)*Ogre::Math::PI/180.f),-local_y_axis)*dir;
        right = right-dir;
        right.y = 0;

        Ogre::Vector3 up = Ogre::Quaternion(Ogre::Radian((fovy*0.5f)*Ogre::Math::PI/180.f),local_x_axis)*dir;
        up = up-dir;
        up.x = 0;
		up.z = 0;

        Ogre::Vector3 down = Ogre::Quaternion(Ogre::Radian((fovy*0.5f)*Ogre::Math::PI/180.f),-local_x_axis)*dir;
        down = down-dir;
        down.x = 0;
		down.z = 0;


        bottom_left = dir + left + down;
        top_left = dir + left + up;
        bottom_right = dir + right + down;
	

		bottom_left = rot * bottom_left;
		top_left = rot * top_left;
		bottom_right = rot * bottom_right;
		
		qDebug() << "dir: x = " << dir.x << " y = " << dir.y << " z = " << dir.z;
		qDebug() << "Botleft: x = " << bottom_left.x << " y = " << bottom_left.y << " z = " << bottom_left.z;
		qDebug() << "Topleft: x = " << top_left.x << " y = " << top_left.y << " z = " << top_left.z;
		qDebug() << "Botright: x = " << bottom_right.x << " y = " << bottom_right.y << " z = " << bottom_right.z;
	}
}

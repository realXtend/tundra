// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Physics_PhysicsContact_h
#define incl_Physics_PhysicsContact_h

#include "Core.h"

#include <QObject>

//! Information about a physics collision. Should be accessible from script
class PhysicsContact : public QObject
{
    Q_OBJECT
    
public:
    //! World space position of the collision
    Q_PROPERTY(Vector3df position READ Position);
    //! World space normal of the collision
    Q_PROPERTY(Vector3df normal READ Normal);
    //! Distance between the colliding surfaces at the time of impact
    Q_PROPERTY(float distance READ Distance);
    //! Impulse of the collision
    Q_PROPERTY(float impulse READ Impulse);
    //! Whether the collision is new, ie. this pair of objects did not collide on the previous simulation step
    Q_PROPERTY(bool newCollision READ NewCollision);
    
    Vector3df Position() const { return position; }
    Vector3df Normal() const { return normal; }
    float Distance() const { return distance; }
    float Impulse() const { return impulse; }
    bool NewCollision() { return newCollision; }
    
    Vector3df position;
    Vector3df normal;
    float distance;
    float impulse;
    bool newCollision;
};

#endif

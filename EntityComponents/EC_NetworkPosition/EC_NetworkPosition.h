// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_EC_NetworkPosition_h
#define incl_EC_NetworkPosition_h

#include "IComponent.h"
#include "RexUUID.h"
#include "Declare_EC.h"
#include "Vector3D.h"
#include "Quaternion.h"
#include "CoreTypes.h"

#include <QtGui/qquaternion.h>
#include <QtGui/qvector3d.h>

//! Represents object position/rotation/velocity data received from network, for clientside inter/extrapolation
/*! Note that currently values are stored in Ogre format axes.
 */
class EC_NetworkPosition : public IComponent
{
    Q_OBJECT
        
    DECLARE_EC(EC_NetworkPosition);

    Q_PROPERTY(QVector3D Position READ GetQPosition WRITE SetQPosition)
    Q_PROPERTY(QQuaternion Orientation READ GetQOrientation WRITE SetQOrientation)

public:
    virtual ~EC_NetworkPosition();
    
    //! Position
    Vector3df position_;
    
    //! Velocity
    Vector3df velocity_;
           
    //! Acceleration
    Vector3df accel_;
    
    //! Orientation
    Quaternion orientation_;
    
    //! Rotational velocity;
    Vector3df rotvel_;
    
    //! Age of current update from network
    f64 time_since_update_;      
    
    //! Previous update interval
    f64 time_since_prev_update_;         
    
    //! Damped position
    Vector3df damped_position_; 
    
    //! Damped orientation
    Quaternion damped_orientation_;
    
    //! Whether update is first
    bool first_update;        
            
    //! Finished an update
    void Updated();
    
    //! Set position forcibly, for example in editing tools
    void SetPosition(const Vector3df& position);
    
    //! Set orientation forcibly, for example in editing tools
    void SetOrientation(const Quaternion& orientation);

    //! experimental accessors that use the new 3d vector etc types in Qt 4.6, for qproperties
    QVector3D GetQPosition() const;
    void SetQPosition(const QVector3D newpos);

    QQuaternion GetQOrientation() const;
    void SetQOrientation(const QQuaternion newort);

private:
    EC_NetworkPosition(IModule* module);        

    //! Disable position damping, called after setting position forcibly
    void NoPositionDamping();

    //! Disable orientation damping, called after setting orientation forcibly
    void NoOrientationDamping();
     
    //! Disable acceleration/velocity, called after setting position forcibly
    void NoVelocity();

    //! Disable rotational , called after setting orientation forcibly
    void NoRotationVelocity();
};

#endif
// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_EC_NetworkPosition_h
#define incl_EC_NetworkPosition_h

#include "ComponentInterface.h"
#include "Foundation.h"
#include "RexUUID.h"

namespace RexLogic
{
    //! Represents object position/rotation/velocity data received from network, for clientside inter/extrapolation
    /*! Note that currently values are stored in Ogre format axes.
     */ 
    class EC_NetworkPosition : public Foundation::ComponentInterface
    {
        DECLARE_EC(EC_NetworkPosition);
    public:
        virtual ~EC_NetworkPosition();
        
        //! Position
        Core::Vector3df position_;
        
        //! Velocity
        Core::Vector3df velocity_;
               
        //! Acceleration
        Core::Vector3df accel_;
        
        //! Orientation
        Core::Quaternion rotation_;
        
        //! Rotational velocity;
        Core::Vector3df rotvel_;
        
        //! Age of current update from network
        Core::f64 time_since_update_;      
        
        //! Previous update interval
        Core::f64 time_since_prev_update_;         
        
        //! Damped position
        Core::Vector3df damped_position_; 
        
        //! Damped orientation
        Core::Quaternion damped_rotation_;
        
        //! Whether update is first
        bool first_update;        
                
        //! Finished an update
        void Updated();
        
        //! Disable position damping, call after updating position
        void NoPositionDamping();

        //! Disable rotation damping, call after updating rotation
        void NoRotationDamping();
         
    private:
        EC_NetworkPosition(Foundation::ModuleInterface* module);        
    };
}

#endif
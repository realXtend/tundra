// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_EC_NetworkPosition_h
#define incl_EC_NetworkPosition_h

#include "Foundation.h"
#include "ComponentInterface.h"
#include "Foundation.h"
#include "RexUUID.h"
#include "RexLogicModuleApi.h"

namespace RexLogic
{
    //! Represents object position/rotation/velocity data received from network, for clientside inter/extrapolation
    /*! Note that currently values are stored in Ogre format axes.
     */ 
    class REXLOGIC_MODULE_API EC_NetworkPosition : public Foundation::ComponentInterface
    {
        Q_OBJECT
            
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
        Core::Quaternion orientation_;
        
        //! Rotational velocity;
        Core::Vector3df rotvel_;
        
        //! Age of current update from network
        Core::f64 time_since_update_;      
        
        //! Previous update interval
        Core::f64 time_since_prev_update_;         
        
        //! Damped position
        Core::Vector3df damped_position_; 
        
        //! Damped orientation
        Core::Quaternion damped_orientation_;
        
        //! Whether update is first
        bool first_update;        
                
        //! Finished an update
        void Updated();
        
        //! Set position forcibly, for example in editing tools
        void SetPosition(const Core::Vector3df& position);
        
        //! Set orientation forcibly, for example in editing tools
        void SetOrientation(const Core::Quaternion& orientation);
                
    private:
        EC_NetworkPosition(Foundation::ModuleInterface* module);        

        //! Disable position damping, called after setting position forcibly
        void NoPositionDamping();

        //! Disable orientation damping, called after setting orientation forcibly
        void NoOrientationDamping();
         
        //! Disable acceleration/velocity, called after setting position forcibly
        void NoVelocity();

        //! Disable rotational , called after setting orientation forcibly
        void NoRotationVelocity();
    };
}

#endif
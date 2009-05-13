// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_EC_NetworkPosition_h
#define incl_EC_NetworkPosition_h

#include "ComponentInterface.h"
#include "Foundation.h"
#include "RexUUID.h"

namespace RexLogic
{
    //! Represents object position/rotation/velocity data received from network, for clientside inter/extrapolation
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
        
        //! Age of last update from network
        Core::f64 time_since_update_;      
         
    private:
        EC_NetworkPosition(Foundation::ModuleInterface* module);        
    };
}

#endif
// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "EC_NetworkPosition.h"

namespace RexLogic
{
    EC_NetworkPosition::EC_NetworkPosition(Foundation::ModuleInterface* module) : Foundation::ComponentInterface(module->GetFramework()),
        time_since_update_(0.0),
        time_since_prev_update_(0.001),
        first_update(true)
    {        
    }

    EC_NetworkPosition::~EC_NetworkPosition()
    {
    }

    void EC_NetworkPosition::Updated()
    {                   
        // See if updated many times on the same frame, don't "update" in that case
        if (time_since_update_ != 0.0)
        {     
            time_since_prev_update_ = time_since_update_;
            time_since_update_ = 0.0;
        }
        
        if (first_update)
        {
            first_update = false;
            NoPositionDamping();
            NoRotationDamping();
        }
    }
    
    void EC_NetworkPosition::NoPositionDamping()
    {
        damped_position_ = position_;
    }
    
    void EC_NetworkPosition::NoRotationDamping()
    {
        damped_rotation_ = rotation_;
    }
}
/// @file Environment.cpp
/// @brief Manages environment-related reX-logic, e.g. world time and lightning.
/// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "RexLogicModule.h"
#include "Environment.h"
#include "Foundation.h"
#include "EC_OgreEnvironment.h"
#include "Entity.h"

namespace RexLogic
{

Environment::Environment(RexLogicModule *owner)
:owner_(owner)
{
}

Environment::~Environment()
{
}
   
bool Environment::HandleOSNE_SimulatorViewerTimeMessage(OpenSimProtocol::NetworkEventInboundData *data)
{
    NetInMessage &msg = *data->message;
    msg.ResetReading();

    usecSinceStart_ = (time_t)msg.ReadU64();
    secPerDay_ = msg.ReadU32(); // seems to be zero, at least with 0.4 server
    secPerYear_ = msg.ReadU32(); // seems to be zero, at least with 0.4 server
    sunDirection_ = msg.ReadVector3();
    sunPhase_ = msg.ReadF32(); // seems to be zero, at least with 0.4 server
    sunAngVelocity_ = msg.ReadVector3();
    
    if (!entity_)
        return false;

    Foundation::ComponentPtr component = entity_->GetComponent("EC_OgreEnvironment");
    if (!component)
        return false;
    
    // Update the sunlight direction and angle velocity.
    OgreRenderer::EC_OgreEnvironment &env = *checked_static_cast<OgreRenderer::EC_OgreEnvironment*>
        (component.get());
        
    env.SetSunDirection(sunDirection_);
    // Set somekind of ambient light, so that the lights are visible.
    ///\todo Find a good default value
    env.SetAmbientLightColor(Core::Color(0.75f, 0.75f, 0.75f, 0.1f));

    return false;    
}

}

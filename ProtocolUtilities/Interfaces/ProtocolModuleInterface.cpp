// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "ProtocolModuleInterface.h"

namespace ProtocolUtilities
{

    NetworkingRegisteredEvent::NetworkingRegisteredEvent(const boost::weak_ptr<ProtocolUtilities::ProtocolModuleInterface> pModule)
    :currentProtocolModule(pModule)
    {
    }
    
    NetworkingRegisteredEvent::~NetworkingRegisteredEvent()
    {
    }

    WorldStreamReadyEvent::WorldStreamReadyEvent(const boost::shared_ptr<ProtocolUtilities::WorldStream> currentWorldStream)
    :WorldStream(currentWorldStream)
    {
    }

    WorldStreamReadyEvent::~WorldStreamReadyEvent()
    {
    }

}

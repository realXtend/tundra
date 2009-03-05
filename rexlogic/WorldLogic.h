// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_WorldLogic_h
#define incl_WorldLogic_h

#include "WorldLogicInterface.h"
#include "NetworkEventHandler.h"

class WorldLogic : public Foundation::WorldLogicInterface
{
public:
    WorldLogic(Foundation::Framework *framework);
    virtual ~WorldLogic();
    
    NetworkEventHandler *GetNetworkHandler() const { return network_handler_; }
private:
    Foundation::Framework *framework_;
    NetworkEventHandler *network_handler_;   
};

#endif

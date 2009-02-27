// For conditions of distribution and use, see copyright notice in license.txt

#ifndef __incl_WorldLogic_h__
#define __incl_WorldLogic_h__

#include "WorldLogicInterface.h"
#include "NetworkEventHandler.h"

class WorldLogic : public Foundation::WorldLogicInterface
{
public:
    WorldLogic(Foundation::Framework *framework);
    virtual ~WorldLogic();
    
    NetworkEventHandler *getNetworkHandler() const { return mNetworkHandler; }
private:
    Foundation::Framework *mFramework;
    NetworkEventHandler *mNetworkHandler;   
};

#endif



#ifndef __incl_NetworkEventHandler_h__
#define __incl_NetworkEventHandler_h__


#include "ComponentInterface.h"
#include "RexEntity.h"
#include "Foundation.h"

class NetworkEventHandler
{
public:
    NetworkEventHandler(Foundation::Framework *framework);
    virtual ~NetworkEventHandler();
    
    void handleNetworkMessage(std::string &packetdata);
    
    void registerForNetworkMessages(const std::string componentname, std::vector<std::string> networkmessagenames);
    void unregisterForNetworkMessage(const std::string componentname, std::vector<std::string> networkmessagenames);
    
private:
    void handleRegionHandshake(std::string &packetdata);
    void handleTeleportStart(std::string &packetdata);
    void handleTeleportLocal(std::string &packetdata);
    void handleAgentUpdate(std::string &packetdata);
    void handleImprovedTerseObjectUpdate(std::string &packetdata);
    void handleGenericMessage(std::string &packetdata);
    void handleObjectUpdate(std::string &packetdata);  
    void handleGenericMessageExtraEntityData(std::string &packetdata);

    void createAvatar(std::string &packetdata);
    void createEntity(std::string &packetdata);    
    void createLight(std::string &packetdata);
    void createSound(std::string &packetdata);
    void createSkybox(std::string &packetdata);
    void createTerrain(std::string &packetdata);

    void updateComponents(const std::string &packetname, const std::string &packetdata);

    std::vector<std::string> getRegisteredEntityComponentClasses(const std::string &networkmessageid);


    typedef std::pair<std::string, std::vector<std::string> > NetworkMessagePair;
    std::map<std::string, std::vector<std::string> > mNetworkMessageMap;


    Foundation::Framework *mFramework;
};
 

#endif

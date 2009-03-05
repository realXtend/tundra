// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_NetworkEventHandler_h
#define incl_NetworkEventHandler_h


#include "ComponentInterface.h"
#include "RexEntity.h"
#include "Foundation.h"

class NetworkEventHandler
{
public:
    NetworkEventHandler(Foundation::Framework *framework);
    virtual ~NetworkEventHandler();
    
    void HandleNetworkMessage(std::string &packetdata);
    
    void RegisterForNetworkMessages(const std::string componentname, std::vector<std::string> networkmessagenames);
    void UnregisterForNetworkMessage(const std::string componentname, std::vector<std::string> networkmessagenames);
    
private:
    void HandleRegionHandshake(std::string &packetdata);
    void HandleTeleportStart(std::string &packetdata);
    void HandleTeleportLocal(std::string &packetdata);
    void HandleAgentUpdate(std::string &packetdata);
    void HandleImprovedTerseObjectUpdate(std::string &packetdata);
    void HandleGenericMessage(std::string &packetdata);
    void HandleObjectUpdate(std::string &packetdata);  
    void HandleGenericMessageExtraEntityData(std::string &packetdata);

    void CreateAvatar(std::string &packetdata);
    void CreateEntity(std::string &packetdata);    
    void CreateLight(std::string &packetdata);
    void CreateSound(std::string &packetdata);
    void CreateSkybox(std::string &packetdata);
    void CreateTerrain(std::string &packetdata);

    void UpdateComponents(const std::string &packetname, const std::string &packetdata);

    std::vector<std::string> GetRegisteredEntityComponentClasses(const std::string &networkmessageid);


    typedef std::pair<std::string, std::vector<std::string> > NetworkMessagePair;
    std::map<std::string, std::vector<std::string> > network_message_map_;


    Foundation::Framework *framework_;
};


#endif

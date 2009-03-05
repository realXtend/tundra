// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "NetworkEventHandler.h"
//#include "ComponentInterface.h"
//#include "ComponentManager.h"
//#include "EntityManager.h"


NetworkEventHandler::NetworkEventHandler(Foundation::Framework *framework)
{
    framework_ = framework;
}

NetworkEventHandler::~NetworkEventHandler()
{
}

void NetworkEventHandler::HandleNetworkMessage(std::string &packetdata)
{
    const std::string mestype = packetdata; // fixme, get real messagetype from packetdata
   
    // What is this network message? Call the right function to handle it.
    // Fixme, implement this with something else than a huge if-else jungle.
    // if(mestype == "ObjectUpdate")
        HandleObjectUpdate(packetdata);
}







void NetworkEventHandler::HandleObjectUpdate(std::string &packetdata)
{
    Core::entity_id_t tempentityid = 0; // fixme, get entity from packetdata

    Foundation::EntityPtr entity = framework_->GetEntityManager()->getEntity(tempentityid);
    if(entity == NULL)
        CreateEntity(packetdata);
    
    UpdateComponents("ObjectUpdate",packetdata);    
}

void NetworkEventHandler::HandleGenericMessageExtraEntityData(std::string &packetdata)
{
    // Fixme, this is the same as above!!! ARE ALL NETWORK MESSAGES HANDLED THEN BY ONE FUNCTION?!?!?!?!?!??!  

    Core::entity_id_t tempentityid = 0; // fixme, get entity from packetdata

    Foundation::EntityPtr entity = framework_->GetEntityManager()->getEntity(tempentityid);
    if(entity == NULL)
        CreateEntity(packetdata);
    
    UpdateComponents("GeneralMessage_ExtraEntityData",packetdata);
}




void NetworkEventHandler::CreateEntity(std::string &packetdata)
{
    Core::StringVector components;
    
    Foundation::EntityPtr entity = framework_->GetEntityManager()->createEntity(components);
}

void NetworkEventHandler::UpdateComponents(const std::string &packetname, const std::string &packetdata)
{
    /*
    Handle networkdata affecting entitycomponents.    
    
    Two choices:
    1) Entitycomponentclasses register for network messages they want to handle.
    2) Make a hardcoded list specific to rex: network message-entitycomponentclasses that want to handle it.
    
    ObjectUpdate : generalproperties, ...
    GeneralMesssage_ExtraEntityData: EC_Collision, EC_SelectPriority, ES_ServerScript, EC_SpatialSound
    
    Component should first decide what it wants to do with the data. It might:
    - if data requires a component instance which doesn't exist, create
    - if data requires changing component instance variable values, update
    - if data requires that component instance should be deleted, delete    
    
    The code is something like this:
    */ 
    
    Foundation::ComponentInterfacePtr tempcomponent;
    Core::entity_id_t tempentityid = 0; // "fixme, get entityid from packetdata
    
    std::vector<std::string> componenttypes = GetRegisteredEntityComponentClasses(packetname);
    std::vector<std::string>::iterator itr;
    for ( itr = componenttypes.begin(); itr < componenttypes.end(); ++itr )
    {
        tempcomponent = framework_->GetComponentManager()->GetComponent(tempentityid,*itr);
        if(tempcomponent.get() == NULL)
            tempcomponent = framework_->GetComponentManager()->CreateComponent(*itr); // fixme, tempentityid is also necessary for component 
    
        tempcomponent->HandleNetworkData(packetdata);        
    }
}








// Map which components are registered for which network messages
void NetworkEventHandler::RegisterForNetworkMessages(const std::string componentname, std::vector<std::string> networkmessagenames)
{
    std::vector<std::string>::iterator itr;
      
    for ( itr = networkmessagenames.begin(); itr < networkmessagenames.end(); ++itr )
    {
        if(network_message_map_.count(*itr) == 0)
        {
            std::vector<std::string> componentnames;
            
            componentnames.push_back(componentname);
            network_message_map_.insert( NetworkMessagePair(*itr,componentnames) );
        }
        else
        {
            std::vector<std::string> componentnames = network_message_map_[*itr];
            
            componentnames.push_back(componentname);
            network_message_map_[*itr] = componentnames;
        }
    }
}

void NetworkEventHandler::UnregisterForNetworkMessage(const std::string componentname, std::vector<std::string> networkmessagenames)
{
    // fixme, not implemented
}

std::vector<std::string> NetworkEventHandler::GetRegisteredEntityComponentClasses(const std::string &networkmessageid)
{
    if(network_message_map_.count(networkmessageid) > 0)
    {
        return network_message_map_[networkmessageid];
    }
    else
    {    
        std::vector<std::string> empty;
        return empty; 
    } 
}
// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "NetworkEventHandler.h"
//#include "ComponentInterface.h"
//#include "ComponentManager.h"
//#include "EntityManager.h"


NetworkEventHandler::NetworkEventHandler(Foundation::Framework *framework)
{
    mFramework = framework;
}

NetworkEventHandler::~NetworkEventHandler()
{
}

void NetworkEventHandler::handleNetworkMessage(std::string &packetdata)
{
    const std::string mestype = packetdata; // fixme, get real messagetype from packetdata
   
    // What is this network message? Call the right function to handle it.
    // Fixme, implement this with something else than a huge if-else jungle.
    // if(mestype == "ObjectUpdate")
        handleObjectUpdate(packetdata);
}







void NetworkEventHandler::handleObjectUpdate(std::string &packetdata)
{
    Core::entity_id_t tempentityid = 0; // fixme, get entity from packetdata

    Foundation::EntityPtr entity = mFramework->getEntityManager()->getEntity(tempentityid);
    if(entity == NULL)
        createEntity(packetdata);
    
    updateComponents("ObjectUpdate",packetdata);    
}

void NetworkEventHandler::handleGenericMessageExtraEntityData(std::string &packetdata)
{
    // Fixme, this is the same as above!!! ARE ALL NETWORK MESSAGES HANDLED THEN BY ONE FUNCTION?!?!?!?!?!??!  

    Core::entity_id_t tempentityid = 0; // fixme, get entity from packetdata

    Foundation::EntityPtr entity = mFramework->getEntityManager()->getEntity(tempentityid);
    if(entity == NULL)
        createEntity(packetdata);
    
    updateComponents("GeneralMessage_ExtraEntityData",packetdata);
}




void NetworkEventHandler::createEntity(std::string &packetdata)
{
    Core::StringVector components;
    
    Foundation::EntityPtr entity = mFramework->getEntityManager()->createEntity(components);
}

void NetworkEventHandler::updateComponents(const std::string &packetname, const std::string &packetdata)
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
    
    std::vector<std::string> componenttypes = getRegisteredEntityComponentClasses(packetname);
    std::vector<std::string>::iterator itr;
    for ( itr = componenttypes.begin(); itr < componenttypes.end(); ++itr )
    {
        tempcomponent = mFramework->getComponentManager()->getComponent(tempentityid,*itr);
        if(tempcomponent.get() == NULL)
            tempcomponent = mFramework->getComponentManager()->createComponent(*itr); // fixme, tempentityid is also necessary for component 
    
        tempcomponent->handleNetworkData(packetdata);        
    }
}








// Map which components are registered for which network messages
void NetworkEventHandler::registerForNetworkMessages(const std::string componentname, std::vector<std::string> networkmessagenames)
{
    std::vector<std::string>::iterator itr;
      
    for ( itr = networkmessagenames.begin(); itr < networkmessagenames.end(); ++itr )
    {
        if(mNetworkMessageMap.count(*itr) == 0)
        {
            std::vector<std::string> componentnames;
            
            componentnames.push_back(componentname);
            mNetworkMessageMap.insert( NetworkMessagePair(*itr,componentnames) );
        }
        else
        {
            std::vector<std::string> componentnames = mNetworkMessageMap[*itr];
            
            componentnames.push_back(componentname);
            mNetworkMessageMap[*itr] = componentnames;
        }
    }
}

void NetworkEventHandler::unregisterForNetworkMessage(const std::string componentname, std::vector<std::string> networkmessagenames)
{
    // fixme, not implemented
}

std::vector<std::string> NetworkEventHandler::getRegisteredEntityComponentClasses(const std::string &networkmessageid)
{
    if(mNetworkMessageMap.count(networkmessageid) > 0)
    {
        return mNetworkMessageMap[networkmessageid];
    }
    else
    {    
        std::vector<std::string> empty;
        return empty; 
    } 
}
// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_TundraLogicModule_SyncManager_h
#define incl_TundraLogicModule_SyncManager_h

#include "SceneManager.h"

#include <QObject>

#include <set>

namespace Foundation
{
    class Framework;
}

namespace KristalliProtocol
{
    struct UserConnection;
}

namespace TundraLogic
{

class TundraLogicModule;

class SyncManager : public QObject
{
    Q_OBJECT
    
public:
    //! Constructor
    SyncManager(TundraLogicModule* owner, Foundation::Framework* fw);
    
    //! Destructor
    ~SyncManager();
    
    //! Register to entity/component change signals and start syncing them
    void RegisterToScene(Scene::ScenePtr scene);
    
    //! Send pending sync messages for one frame
    void Update();
    
    //! Replicate the whole scene to a new user
    void NewUserConnected(KristalliProtocol::UserConnection* connection);
    
private:
    //! Owning module
    TundraLogicModule* owner_;
    
    //! Framework pointer
    Foundation::Framework* framework_;
    
    //! Known entities within the scene. If an unknown entity changes, it will be sent as a CreateEntity message
    std::set<entity_id_t> knownEntities_;
    
    //! Locally dirty entities (will be replicated)
    std::set<entity_id_t> localDirtyEntities_;
};

}

#endif


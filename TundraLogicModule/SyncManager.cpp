// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "SyncManager.h"

namespace TundraLogic
{

SyncManager::SyncManager(TundraLogicModule* owner, Foundation::Framework* fw) :
    owner_(owner),
    framework_(fw)
{
}

SyncManager::~SyncManager()
{
}

void SyncManager::RegisterToScene(Scene::ScenePtr scene)
{
    knownEntities_.clear();
    localDirtyEntities_.clear();
}

void SyncManager::NewUser(KristalliProtocol::UserConnection* connection)
{
}

void SyncManager::Update()
{
}

}

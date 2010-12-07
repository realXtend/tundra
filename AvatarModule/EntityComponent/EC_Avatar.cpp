// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "EntityComponent/EC_Avatar.h"
#include "AvatarModule.h"
#include "Avatar/AvatarHandler.h"
#include "AssetEvents.h"
#include "AssetInterface.h"
#include "AssetServiceInterface.h"
#include "EventManager.h"
#include "RexTypes.h"
#include "EC_Mesh.h"
#include "EC_AnimationController.h"
#include "EC_Placeable.h"
#include "EC_AvatarAppearance.h"
#include "AssetAPI.h"
#include "IAssetTransfer.h"
#include "AvatarDescAsset.h"

#include "LoggingFunctions.h"
DEFINE_POCO_LOGGING_FUNCTIONS("EC_Avatar")

#include "MemoryLeakCheck.h"

using namespace RexTypes;

EC_Avatar::EC_Avatar(IModule* module) :
    IComponent(module->GetFramework()),
    appearanceId(this, "Appearance ref", ""),
    avatar_handler_(checked_static_cast<Avatar::AvatarModule*>(module)->GetAvatarHandler()),
    appearance_tag_(0)
{
    EventManager *event_manager = framework_->GetEventManager().get();
    if(event_manager)
    {
        event_manager->RegisterEventSubscriber(this, 99);
        asset_event_category_ = event_manager->QueryEventCategory("Asset");
    }
    else
    {
        LogWarning("Event manager was not valid.");
    }
    
    connect(this, SIGNAL(OnAttributeChanged(IAttribute*, AttributeChange::Type)),
        this, SLOT(AttributeUpdated(IAttribute*)));
}

EC_Avatar::~EC_Avatar()
{
}
/*
bool EC_Avatar::HandleEvent(event_category_id_t category_id, event_id_t event_id, IEventData *data)
{
    if(category_id == asset_event_category_)
    {
        if(event_id == Asset::Events::ASSET_READY)
        {
            return HandleAssetReady(data);
        }
    }
    return false;
}
*/
void EC_Avatar::OnAvatarAppearanceLoaded(IAssetTransfer *transfer)
{
    if (!transfer)
        return;

    Scene::Entity* entity = GetParentEntity();
    if (!entity)
        return;

    if (!avatar_handler_)
        return;

    AvatarDescAssetPtr avatarAsset = boost::dynamic_pointer_cast<AvatarDescAsset>(transfer->asset);
    if (!avatarAsset.get())
        return;

    // Create components the avatar needs, with network sync disabled, if they don't exist yet
    // Note: the mesh & avatarappearance are created as non-syncable on purpose, as each client's EC_Avatar should execute this code upon receiving the appearance
    ComponentPtr mesh = entity->GetOrCreateComponent(EC_Mesh::TypeNameStatic(), AttributeChange::LocalOnly, false);
    entity->GetOrCreateComponent(EC_AvatarAppearance::TypeNameStatic(), AttributeChange::LocalOnly, false);
    EC_Mesh *mesh_ptr = checked_static_cast<EC_Mesh*>(mesh.get());
    // Attach to placeable if not yet attached
    if (mesh_ptr && !mesh_ptr->GetPlaceable())
        mesh_ptr->SetPlaceable(entity->GetComponent(EC_Placeable::TypeNameStatic()));
    
    if (transfer->rawAssetData.size() > 0)
        avatar_handler_->SetupECAvatar(entity->GetId(), &transfer->rawAssetData[0], transfer->rawAssetData.size());
}

void EC_Avatar::AttributeUpdated(IAttribute *attribute)
{
    if (attribute == &appearanceId)
    {
        QString ref = appearanceId.Get().trimmed();
        if (ref.isEmpty())
            return;

        AssetTransferPtr transfer = GetFramework()->Asset()->RequestAsset(ref.toStdString().c_str(), ASSETTYPENAME_GENERIC_AVATAR_XML.c_str());
        if (transfer.get())
        {
            connect(transfer.get(), SIGNAL(Loaded(IAssetTransfer*)), this, SLOT(OnAvatarAppearanceLoaded(IAssetTransfer *)));
        }
/*            
        boost::shared_ptr<Foundation::AssetServiceInterface> asset_service = 
            GetFramework()->GetServiceManager()->GetService<Foundation::AssetServiceInterface>(Service::ST_Asset).lock();
        if (!asset_service)
            return;
        request_tag_t tag = asset_service->RequestAsset(ref.toStdString(), ASSETTYPENAME_GENERIC_AVATAR_XML);
        if (tag)
            appearance_tag_ = tag;
*/
    }
}


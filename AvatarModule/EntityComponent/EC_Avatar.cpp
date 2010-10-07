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
    Foundation::EventManager *event_manager = framework_->GetEventManager().get();
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

bool EC_Avatar::HandleAssetReady(IEventData* data)
{
    Scene::Entity* entity = GetParentEntity();
    if (!entity)
        return false;
    
    Asset::Events::AssetReady *assetReady = checked_static_cast<Asset::Events::AssetReady*>(data);
    request_tag_t tag = assetReady->tag_;
    if (tag == appearance_tag_)
    {
        Foundation::AssetPtr asset = assetReady->asset_;
        if (!asset)
            return false;
        if (!avatar_handler_)
            return false;
        
        // Create components the avatar needs, with network sync disabled, if they don't exist yet
        //! \todo we want the animationcontroller to be synced, once it has attributes
        ComponentPtr mesh = entity->GetOrCreateComponent(EC_Mesh::TypeNameStatic(), AttributeChange::LocalOnly, false);
        ComponentPtr anim = entity->GetOrCreateComponent(EC_AnimationController::TypeNameStatic(), AttributeChange::LocalOnly, false);
        entity->GetOrCreateComponent(EC_AvatarAppearance::TypeNameStatic(), AttributeChange::LocalOnly, false);
        // Associate the animationcontroller with the mesh
        EC_AnimationController* anim_ptr = checked_static_cast<EC_AnimationController*>(anim.get());
        EC_Mesh* mesh_ptr = checked_static_cast<EC_Mesh*>(mesh.get());
        if ((anim_ptr) && (mesh_ptr))
        {
            if (anim_ptr->GetMeshEntity() != mesh_ptr)
                anim_ptr->SetMeshEntity(mesh_ptr);
            
            // Attach to placeable if not yet attached
            if (!mesh_ptr->GetPlaceable())
                mesh_ptr->SetPlaceable(entity->GetComponent(EC_Placeable::TypeNameStatic()));
        }
        
        avatar_handler_->SetupECAvatar(entity->GetId(), asset->GetData(), asset->GetSize());
        appearance_tag_ = 0;
        return true;
    }
    else
        return false;
}

void EC_Avatar::AttributeUpdated(IAttribute *attribute)
{
    if (attribute == &appearanceId)
    {
        QString ref = appearanceId.Get();
        if (!ref.length())
            return;
            
        boost::shared_ptr<Foundation::AssetServiceInterface> asset_service = 
            GetFramework()->GetServiceManager()->GetService<Foundation::AssetServiceInterface>(Foundation::Service::ST_Asset).lock();
        if (!asset_service)
            return;
        request_tag_t tag = asset_service->RequestAsset(ref.toStdString(), ASSETTYPENAME_GENERIC_AVATAR_XML);
        if (tag)
            appearance_tag_ = tag;
    }
}


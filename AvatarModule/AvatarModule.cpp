// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "AvatarModule.h"
#include "EventManager.h"
#include "AvatarEvents.h"
#include "NetworkEvents.h"

#include "EntityComponent/EC_AvatarAppearance.h"
#include "EntityComponent/EC_OpenSimAvatar.h"
#include "EntityComponent/EC_Controllable.h"
#include "EntityComponent/EC_Avatar.h"

#include "EC_NetworkPosition.h"
#ifdef EC_HoveringWidget_ENABLED
#include "EC_HoveringWidget.h"
#endif

#include "Avatar/AvatarHandler.h"
#include "Avatar/AvatarControllable.h"
#include "AvatarEditing/AvatarEditor.h"
#include "AvatarEditing/AvatarSceneManager.h"

namespace Avatar
{
    static std::string module_name = "AvatarModule";
    const std::string &AvatarModule::NameStatic() { return module_name; }

    AvatarModule::AvatarModule() :
        QObject(),
        IModule(module_name),
        scene_manager_(0)
    {
        world_stream_.reset();
        uuid_to_local_id_.clear();
    }

    AvatarModule::~AvatarModule()
    {
    }

    void AvatarModule::Load()
    {
        DECLARE_MODULE_EC(EC_AvatarAppearance);
        DECLARE_MODULE_EC(EC_OpenSimAvatar);
        DECLARE_MODULE_EC(EC_NetworkPosition);
        DECLARE_MODULE_EC(EC_Controllable);
        DECLARE_MODULE_EC(EC_Avatar);
#ifdef EC_HoveringWidget_ENABLED
        DECLARE_MODULE_EC(EC_HoveringWidget);
#endif
    }

    void AvatarModule::Initialize()
    {
        event_query_categories_ << "Framework" << "Scene" << "NetworkState" << "Avatar" << "Resource" << "Asset" << "Inventory" << "Input" << "Action";

        avatar_handler_ = AvatarHandlerPtr(new AvatarHandler(this));
        avatar_controllable_ = AvatarControllablePtr(new AvatarControllable(this));
        avatar_editor_ = AvatarEditorPtr(new AvatarEditor(this));
        scene_manager_ = new AvatarSceneManager(this, avatar_editor_.get());
    }

    void AvatarModule::PostInitialize()
    {
        SubscribeToEventCategories();
        scene_manager_->InitScene();

        avatar_context_ = GetFramework()->Input()->RegisterInputContext("Avatar", 100);
        if (avatar_context_)
        {
            connect(avatar_context_.get(), SIGNAL(KeyPressed(KeyEvent*)), SLOT(KeyPressed(KeyEvent*)));
            connect(avatar_context_.get(), SIGNAL(KeyReleased(KeyEvent*)), SLOT(KeyReleased(KeyEvent*)));
        }
    }

    void AvatarModule::Uninitialize()
    {
        avatar_handler_.reset();
        avatar_controllable_.reset();
        avatar_editor_.reset();
        world_stream_.reset();
        uuid_to_local_id_.clear();

        SAFE_DELETE(scene_manager_);
    }

    Scene::EntityPtr AvatarModule::GetAvatarEntity(const RexUUID &uuid)
    {
        if (uuid_to_local_id_.contains(uuid))
            return GetAvatarEntity(uuid_to_local_id_[uuid]);
        else
            return Scene::EntityPtr();
    }

    Scene::EntityPtr AvatarModule::GetAvatarEntity(entity_id_t entity_id)
    {
        Scene::ScenePtr current_scene = GetFramework()->GetDefaultWorldScene();
        if (!current_scene)
            return Scene::EntityPtr();

        Scene::EntityPtr entity = current_scene->GetEntity(entity_id);
        // Allow also EC_Avatar, the new attribute-based component
        if (entity && ((entity->GetComponent("EC_OpenSimAvatar")) || (entity->GetComponent("EC_Avatar"))))
            return entity;
        else
            return Scene::EntityPtr();
    }

    void AvatarModule::RegisterFullId(const RexUUID &full_uuid, entity_id_t entity_id)
    {
        if (!uuid_to_local_id_.contains(full_uuid))
        {
            uuid_to_local_id_[full_uuid] = entity_id;
            Events::SceneRegisterEntityData data(full_uuid, entity_id);
            GetFramework()->GetEventManager()->SendEvent("Avatar", Events::EVENT_REGISTER_UUID_TO_LOCALID, &data);
        }
    }

    void AvatarModule::UnregisterFullId(const RexUUID &full_uuid)
    {
        if (uuid_to_local_id_.remove(full_uuid) > 0)
        {
            Events::SceneRegisterEntityData data(full_uuid);
            GetFramework()->GetEventManager()->SendEvent("Avatar", Events::EVENT_UNREGISTER_UUID_TO_LOCALID, &data);
        }
    }

    void AvatarModule::Update(f64 frametime)
    {
        avatar_handler_->Update(frametime);
        avatar_controllable_->AddTime(frametime);
    }

    bool AvatarModule::HandleEvent(event_category_id_t category_id, event_id_t event_id, IEventData* data)
    {
        bool handled = false;
        QString category = service_category_identifiers_.keys().value(
            service_category_identifiers_.values().indexOf(category_id));
        
        if (category == "Framework" && event_id == Foundation::WORLD_STREAM_READY)
        {
            ProtocolUtilities::WorldStreamReadyEvent *event_data = checked_static_cast<ProtocolUtilities::WorldStreamReadyEvent *>(data);
            if (event_data)
                world_stream_ = event_data->WorldStream;
        }
        else if (category == "NetworkState")
        {
            switch (event_id)
            {
                case ProtocolUtilities::Events::EVENT_SERVER_DISCONNECTED:
                case ProtocolUtilities::Events::EVENT_CONNECTION_FAILED:
                {
                    world_stream_.reset();
                    uuid_to_local_id_.clear();
                    break;
                }
                default:
                    break;
            }
        }
        else if (category == "Avatar")
        {
            switch (event_id)
            {
                case Events::EVENT_REGISTER_UUID_TO_LOCALID:
                {
                    Events::SceneRegisterEntityData *in_data = dynamic_cast<Events::SceneRegisterEntityData*>(data);
                    if (in_data)
                        RegisterFullId(in_data->uuid, in_data->local_id);
                    break;
                }
                case Events::EVENT_UNREGISTER_UUID_TO_LOCALID:
                {
                    Events::SceneRegisterEntityData *in_data = dynamic_cast<Events::SceneRegisterEntityData*>(data);
                    if (in_data)
                        UnregisterFullId(in_data->uuid);
                    break;
                }
                default:
                    break;
            }
        }
        else if (category == "Input")
        {
            handled = avatar_controllable_->HandleInputEvent(event_id, data);
        }
        else if (category == "Action")
        {
            handled = avatar_controllable_->HandleActionEvent(event_id, data);
        }
        else if (category == "Scene")
        {
            handled = avatar_controllable_->HandleSceneEvent(event_id, data);
        }
        else if (category == "Asset")
        {
            handled = avatar_handler_->HandleAssetEvent(event_id, data);
        }
        else if (category == "Inventory")
        {
            handled = avatar_handler_->HandleInventoryEvent(event_id, data);
        }
        else if (category == "Resource")
        {
            handled = avatar_handler_->HandleResourceEvent(event_id, data);
        }
        return handled;
    }

    void AvatarModule::SubscribeToEventCategories()
    {
        service_category_identifiers_.clear();
        foreach (QString category, event_query_categories_)
            service_category_identifiers_[category] = GetFramework()->GetEventManager()->QueryEventCategory(category.toStdString());
    }

    void AvatarModule::KeyPressed(KeyEvent *key)
    {
        if (key->IsRepeat())
            return;

        if (key->HasCtrlModifier() && key->keyCode == Qt::Key_A)
        {
            scene_manager_->ToggleScene();
            return;
        }
    }

    void AvatarModule::KeyReleased(KeyEvent *key)
    {
    
    }
}

extern "C" void POCO_LIBRARY_API SetProfiler(Foundation::Profiler *profiler);
void SetProfiler(Foundation::Profiler *profiler)
{
    Foundation::ProfilerSection::SetProfiler(profiler);
}

using namespace Avatar;
POCO_BEGIN_MANIFEST(IModule)
    POCO_EXPORT_CLASS(AvatarModule)
POCO_END_MANIFEST

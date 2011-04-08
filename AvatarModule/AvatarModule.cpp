// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "AvatarModule.h"
#include "AvatarEvents.h"
#include "Avatar/AvatarHandler.h"
#include "Avatar/AvatarControllable.h"
#include "AvatarEditing/AvatarEditor.h"
#include "AvatarEditing/AvatarSceneManager.h"
#include "ConsoleCommandServiceInterface.h"
#include "EventManager.h"
#ifdef ENABLE_TAIGA_SUPPORT
#include "NetworkEvents.h"
#endif
#include "InputAPI.h"
#include "SceneManager.h"
#include "SceneAPI.h"
#include "AssetAPI.h"
#include "GenericAssetFactory.h"
#include "AvatarDescAsset.h"

#include "EntityComponent/EC_AvatarAppearance.h"

#ifdef ENABLE_TAIGA_SUPPORT
#include "EntityComponent/EC_OpenSimAvatar.h"
#include "EntityComponent/EC_Controllable.h"
#include "EC_NetworkPosition.h"
#endif

#include "EntityComponent/EC_Avatar.h"


#ifdef EC_HoveringWidget_ENABLED
#include "EC_HoveringWidget.h"
#endif



namespace Avatar
{
    static std::string module_name = "AvatarModule";
    const std::string &AvatarModule::NameStatic() { return module_name; }

    AvatarModule::AvatarModule() :
        QObject(),
        IModule(module_name)
        //scene_manager_(0)
    {
#ifdef ENABLE_TAIGA_SUPPORT
        world_stream_.reset();
        uuid_to_local_id_.clear();
#endif
    }

    AvatarModule::~AvatarModule()
    {
    }

    void AvatarModule::Load()
    {
<<<<<<< HEAD
        DECLARE_MODULE_EC(EC_AvatarAppearance);
#ifdef ENABLE_TAIGA_SUPPORT
=======
        //DECLARE_MODULE_EC(EC_AvatarAppearance);
>>>>>>> tundra
        DECLARE_MODULE_EC(EC_OpenSimAvatar);
        DECLARE_MODULE_EC(EC_NetworkPosition);
        DECLARE_MODULE_EC(EC_Controllable);
#endif
        DECLARE_MODULE_EC(EC_Avatar);
#ifdef EC_HoveringWidget_ENABLED
        DECLARE_MODULE_EC(EC_HoveringWidget);
#endif
    }

    void AvatarModule::Initialize()
    {
#ifdef ENABLE_TAIGA_SUPPORT
        event_query_categories_ << "Framework" << "Scene" << "NetworkState" << "Avatar" << "Resource" << "Asset" << "Inventory" << "Input" << "Action";
        avatar_handler_ = AvatarHandlerPtr(new AvatarHandler(this));
        avatar_controllable_ = AvatarControllablePtr(new AvatarControllable(this));
<<<<<<< HEAD
        scene_manager_ = new AvatarSceneManager(this, avatar_editor_.get());
        avatar_editor_ = AvatarEditorPtr(new AvatarEditor(this));
#endif
=======
        avatar_editor_ = AvatarEditorPtr(new AvatarEditor(this));
        
        //! \todo: no UI in Tundra for the avatarscene, so leave it uncreated
        //scene_manager_ = new AvatarSceneManager(this, avatar_editor_.get());
>>>>>>> tundra
    }

    void AvatarModule::PostInitialize()
    {
#ifdef ENABLE_TAIGA_SUPPORT
        SubscribeToEventCategories();
<<<<<<< HEAD
        scene_manager_->InitScene();
#endif
=======
        //if (scene_manager_)
        //    scene_manager_->InitScene();

>>>>>>> tundra
        avatar_context_ = GetFramework()->Input()->RegisterInputContext("Avatar", 100);
        if (avatar_context_)
        {
            connect(avatar_context_.get(), SIGNAL(KeyPressed(KeyEvent*)), SLOT(KeyPressed(KeyEvent*)));
            connect(avatar_context_.get(), SIGNAL(KeyReleased(KeyEvent*)), SLOT(KeyReleased(KeyEvent*)));
        }

        framework_->Asset()->RegisterAssetTypeFactory(AssetTypeFactoryPtr(new GenericAssetFactory<AvatarDescAsset>("GenericAvatarXml")));
        
        RegisterConsoleCommand(Console::CreateCommand("editavatar",
            "Edits the avatar in a specific entity. Usage: editavatar(entityname)",
            Console::Bind(this, &AvatarModule::EditAvatar)));
    }

    void AvatarModule::Uninitialize()
    {
        avatar_handler_.reset();
        avatar_controllable_.reset();
        avatar_editor_.reset();
#ifdef ENABLE_TAIGA_SUPPORT
        world_stream_.reset();
        uuid_to_local_id_.clear();
<<<<<<< HEAD
        SAFE_DELETE(scene_manager_);
#endif
=======

        //SAFE_DELETE(scene_manager_);
>>>>>>> tundra
    }

#ifdef ENABLE_TAIGA_SUPPORT
    Scene::EntityPtr AvatarModule::GetAvatarEntity(const RexUUID &uuid)
    {
        if (uuid_to_local_id_.contains(uuid))
            return GetAvatarEntity(uuid_to_local_id_[uuid]);
        else
            return Scene::EntityPtr();
    }

    Scene::EntityPtr AvatarModule::GetAvatarEntity(entity_id_t entity_id)
    {
        Scene::ScenePtr current_scene = GetFramework()->Scene()->GetDefaultScene();
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
#endif
    void AvatarModule::Update(f64 frametime)
    {
#ifdef ENABLE_TAIGA_SUPPORT
        avatar_handler_->Update(frametime);
        avatar_controllable_->AddTime(frametime);
#endif
    }

    bool AvatarModule::HandleEvent(event_category_id_t category_id, event_id_t event_id, IEventData* data)
    {
#ifdef ENABLE_TAIGA_SUPPORT

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
#else
        return false;
#endif
    }

#ifdef ENABLE_TAIGA_SUPPORT
    void AvatarModule::SubscribeToEventCategories()
    {
        service_category_identifiers_.clear();
        foreach (QString category, event_query_categories_)
            service_category_identifiers_[category] = GetFramework()->GetEventManager()->QueryEventCategory(category.toStdString());
    }
#endif
    void AvatarModule::KeyPressed(KeyEvent *key)
    {
#ifdef ENABLE_TAIGA_SUPPORT
        if (key->IsRepeat())
            return;

        if (key->HasCtrlModifier() && key->keyCode == Qt::Key_A)
        {
            //if (scene_manager_)
            //    scene_manager_->ToggleScene();
            return;
        }
#endif
    }

    void AvatarModule::KeyReleased(KeyEvent *key)
    {
    
    }
    
    Console::CommandResult AvatarModule::EditAvatar(const StringVector &params)
    {
        if (params.size() < 1)
            return Console::ResultFailure("No entity name given");
        
        QString name = QString::fromStdString(params[0]);
        Scene::ScenePtr scene = framework_->Scene()->GetDefaultScene();
        if (!scene)
            return Console::ResultFailure("No scene");
        Scene::EntityPtr entity = scene->GetEntityByName(name);
        if (!entity)
            return Console::ResultFailure("No such entity " + params[0]);
        
        avatar_editor_->SetAvatarEntityName(name);
        avatar_editor_->RebuildEditView();
        
        if (avatar_editor_)
            avatar_editor_->show();
        
        return Console::ResultSuccess();
    }
}

void SetProfiler(Foundation::Profiler *profiler)
{
    Foundation::ProfilerSection::SetProfiler(profiler);
}

extern "C"
{
__declspec(dllexport) void TundraPluginMain(Foundation::Framework *fw)
{
    IModule *module = new Avatar::AvatarModule();
    fw->GetModuleManager()->DeclareStaticModule(module);
}
}

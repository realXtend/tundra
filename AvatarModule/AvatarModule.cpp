// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "AvatarModule.h"
#include "AvatarEditing/AvatarEditor.h"
#include "ConsoleCommandServiceInterface.h"
#include "EventManager.h"
#include "InputAPI.h"
#include "SceneManager.h"
#include "SceneAPI.h"
#include "AssetAPI.h"
#include "GenericAssetFactory.h"
#include "AvatarDescAsset.h"

#include "EntityComponent/EC_Avatar.h"

namespace Avatar
{
    static std::string module_name = "AvatarModule";
    const std::string &AvatarModule::NameStatic() { return module_name; }

    AvatarModule::AvatarModule() :
        QObject(),
        IModule(module_name)
    {
    }

    AvatarModule::~AvatarModule()
    {
    }

    void AvatarModule::Load()
    {
        DECLARE_MODULE_EC(EC_Avatar);
    }

    void AvatarModule::Initialize()
    {
        avatar_editor_ = AvatarEditorPtr(new AvatarEditor(this));
    }

    void AvatarModule::PostInitialize()
    {
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
    }

    void AvatarModule::Update(f64 frametime)
    {
    }

    bool AvatarModule::HandleEvent(event_category_id_t category_id, event_id_t event_id, IEventData* data)
    {
        return false;
    }

    void AvatarModule::KeyPressed(KeyEvent *key)
    {
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
        
        /// \todo Clone the avatar asset for editing
        /// \todo Allow avatar asset editing without an avatar entity in the scene
        avatar_editor_->SetEntityToEdit(entity);
        
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

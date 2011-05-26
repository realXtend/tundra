// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "AvatarModule.h"
#include "AvatarEditing/AvatarEditor.h"
#include "InputAPI.h"
#include "Scene.h"
#include "SceneAPI.h"
#include "AssetAPI.h"
#include "GenericAssetFactory.h"
#include "NullAssetFactory.h"
#include "AvatarDescAsset.h"
#include "ConsoleAPI.h"
#include "IComponentFactory.h"

#include "EntityComponent/EC_Avatar.h"

namespace Avatar
{
    AvatarModule::AvatarModule()
    :IModule("Avatar")
    {
    }

    AvatarModule::~AvatarModule()
    {
    }

    void AvatarModule::Load()
    {
        framework_->Scene()->RegisterComponentFactory(ComponentFactoryPtr(new GenericComponentFactory<EC_Avatar>));

        ///\todo This doesn't need to be loaded in headless server mode.
        // Note: need to register in Initialize(), because in PostInitialize() AssetModule refreshes the local asset storages, and that 
        // would result in inability to create any avatar assets in unloaded state
        if (!framework_->IsHeadless())
            framework_->Asset()->RegisterAssetTypeFactory(AssetTypeFactoryPtr(new GenericAssetFactory<AvatarDescAsset>("Avatar")));
        else
            framework_->Asset()->RegisterAssetTypeFactory(AssetTypeFactoryPtr(new NullAssetFactory("Avatar")));
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

        framework_->Console()->RegisterCommand("editavatar",
            "Edits the avatar in a specific entity. Usage: editavatar(entityname)",
            this, SLOT(EditAvatar(const QString &)));
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

    void AvatarModule::KeyPressed(KeyEvent *key)
    {
    }

    void AvatarModule::KeyReleased(KeyEvent *key)
    {
    
    }
    
    void AvatarModule::EditAvatar(const QString &entityName)
    {
        ScenePtr scene = framework_->Scene()->GetDefaultScene();
        if (!scene)
            return;// ConsoleResultFailure("No scene");
        EntityPtr entity = scene->GetEntityByName(entityName);
        if (!entity)
            return;// ConsoleResultFailure("No such entity " + entityName.toStdString());
        
        /// \todo Clone the avatar asset for editing
        /// \todo Allow avatar asset editing without an avatar entity in the scene
        avatar_editor_->SetEntityToEdit(entity);
        
        if (avatar_editor_)
            avatar_editor_->show();
    }
}

extern "C"
{
__declspec(dllexport) void TundraPluginMain(Framework *fw)
{
    Framework::SetInstance(fw); // Inside this DLL, remember the pointer to the global framework object.
    IModule *module = new Avatar::AvatarModule();
    fw->RegisterModule(module);
}
}

// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "AvatarModule.h"
#include "AvatarEditor.h"
#include "Scene.h"
#include "SceneAPI.h"
#include "AssetAPI.h"
#include "GenericAssetFactory.h"
#include "NullAssetFactory.h"
#include "AvatarDescAsset.h"
#include "ConsoleAPI.h"
#include "IComponentFactory.h"

#include "EC_Avatar.h"

AvatarModule::AvatarModule() : IModule("Avatar")
{
}

AvatarModule::~AvatarModule()
{
    SAFE_DELETE(avatarEditor);
}

void AvatarModule::Load()
{
    framework_->Scene()->RegisterComponentFactory(ComponentFactoryPtr(new GenericComponentFactory<EC_Avatar>));
    if (!framework_->IsHeadless())
        framework_->Asset()->RegisterAssetTypeFactory(AssetTypeFactoryPtr(new GenericAssetFactory<AvatarDescAsset>("Avatar")));
    else
        framework_->Asset()->RegisterAssetTypeFactory(AssetTypeFactoryPtr(new NullAssetFactory("Avatar")));
}

void AvatarModule::Initialize()
{
    avatarEditor = new AvatarEditor(this);
    framework_->Console()->RegisterCommand("editavatar",
        "Edits the avatar in a specific entity. Usage: editavatar(entityname)",
        this, SLOT(EditAvatar(const QString &)));
}

AvatarEditor* AvatarModule::GetAvatarEditor() const
{
    return avatarEditor.data();
}

void AvatarModule::EditAvatar(const QString &entityName)
{
    Scene *scene = framework_->Scene()->MainCameraScene();
    if (!scene)
        return;// ConsoleResultFailure("No scene");
    EntityPtr entity = scene->GetEntityByName(entityName);
    if (!entity)
        return;// ConsoleResultFailure("No such entity " + entityName.toStdString());
    
    /// \todo Clone the avatar asset for editing
    /// \todo Allow avatar asset editing without an avatar entity in the scene
    avatarEditor->SetEntityToEdit(entity);
    
    if (avatarEditor)
        avatarEditor->show();
}

extern "C"
{
DLLEXPORT void TundraPluginMain(Framework *fw)
{
    Framework::SetInstance(fw); // Inside this DLL, remember the pointer to the global framework object.
    IModule *module = new AvatarModule();
    fw->RegisterModule(module);
}
}

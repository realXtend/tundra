// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"

#include "AvatarModule.h"
#include "AvatarEditor.h"

#include "Scene/Scene.h"
#include "SceneAPI.h"
#include "AssetAPI.h"
#include "GenericAssetFactory.h"
#include "NullAssetFactory.h"
#include "AvatarDescAsset.h"
#include "ConsoleAPI.h"
#include "IComponentFactory.h"
#include "UiAPI.h"
#include "UiMainWindow.h"

#include "EC_Avatar.h"

#include "../JavascriptModule/JavascriptModule.h"
#include "AvatarModuleScriptTypeDefines.h"

#include "StaticPluginRegistry.h"

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
    {
        framework_->Asset()->RegisterAssetTypeFactory(AssetTypeFactoryPtr(new GenericAssetFactory<AvatarDescAsset>("Avatar", ".avatar")));
        framework_->Asset()->RegisterAssetTypeFactory(AssetTypeFactoryPtr(new BinaryAssetFactory("AvatarAttachment", ".attachment")));
    }
    else
    {
        framework_->Asset()->RegisterAssetTypeFactory(AssetTypeFactoryPtr(new NullAssetFactory("Avatar", ".avatar")));
        framework_->Asset()->RegisterAssetTypeFactory(AssetTypeFactoryPtr(new NullAssetFactory("AvatarAttachment", ".attachment")));
    }
}

void AvatarModule::Initialize()
{
    framework_->Console()->RegisterCommand("editavatar",
        "Edits the avatar in a specific entity. Usage: editavatar(entityname)",
        this, SLOT(EditAvatarConsole(const QString &)));

    JavascriptModule *javascriptModule = framework_->GetModule<JavascriptModule>();
    if (javascriptModule)
        connect(javascriptModule, SIGNAL(ScriptEngineCreated(QScriptEngine*)), SLOT(OnScriptEngineCreated(QScriptEngine*)));
    else
        LogWarning("AvatarModule: JavascriptModule not present, AvatarModule usage from scripts will be limited!");
}

AvatarEditor* AvatarModule::GetAvatarEditor() const
{
    return avatarEditor.data();
}

void AvatarModule::EditAvatar(const QString &entityName)
{
    Scene *scene = framework_->Scene()->MainCameraScene();
    if (!scene)
    {
        LogError("AvatarModule::EditAvatar: No scene");
        return;
    }
    EntityPtr entity = scene->GetEntityByName(entityName);
    if (!entity)
    {
        LogError("No such entity " + entityName.toStdString());
        return;
    }

    /// \todo Clone the avatar asset for editing
    /// \todo Allow avatar asset editing without an avatar entity in the scene
    avatarEditor->SetEntityToEdit(entity);
}

void AvatarModule::ToggleAvatarEditorWindow()
{
    if (avatarEditor)
    {
        avatarEditor->setVisible(!avatarEditor->isVisible());
        if (!avatarEditor->isVisible())
        {
            /// \todo Save window position
            avatarEditor->close();
        }
        return;
    }

    avatarEditor = new AvatarEditor(framework_, framework_->Ui()->MainWindow());
    avatarEditor->setAttribute(Qt::WA_DeleteOnClose);
    avatarEditor->setWindowFlags(Qt::Tool);
    // \ todo Load window position
    avatarEditor->show();
}

void AvatarModule::EditAvatarConsole(const QString &entityName)
{
    ToggleAvatarEditorWindow();
    EditAvatar(entityName);
}

void AvatarModule::OnScriptEngineCreated(QScriptEngine *engine)
{
    RegisterAvatarModuleMetaTypes(engine);
}

extern "C"
{
#ifndef ANDROID
DLLEXPORT void TundraPluginMain(Framework *fw)
#else
DEFINE_STATIC_PLUGIN_MAIN(AvatarModule)
#endif
{
    Framework::SetInstance(fw); // Inside this DLL, remember the pointer to the global framework object.
    IModule *module = new AvatarModule();
    fw->RegisterModule(module);
}
}

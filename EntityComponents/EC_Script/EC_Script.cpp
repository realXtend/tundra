// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include <QList>
#include "MemoryLeakCheck.h"
#include "EC_Script.h"
#include "IScriptInstance.h"

#include "IAttribute.h"
#include "IAssetTransfer.h"
#include "Entity.h"
#include "AssetRefListener.h"

#include "LoggingFunctions.h"
DEFINE_POCO_LOGGING_FUNCTIONS("EC_Script")

EC_Script::~EC_Script()
{
    SAFE_DELETE(scriptInstance_);
}

void EC_Script::SetScriptInstance(IScriptInstance *instance)
{
    // If we already have a script instance, unload and delete it.
    if (scriptInstance_)
    {
        scriptInstance_->Unload();
        SAFE_DELETE(scriptInstance_);
    }

    scriptInstance_ = instance;
}

void EC_Script::Run(const QString &name)
{
    // This function (EC_Script::Run) is invoked on the Entity Action RunScript(scriptName). To
    // allow the user to differentiate between multiple instances of EC_Script in the same entity, the first
    // parameter of RunScript allows the user to specify which EC_Script to run. So, first check
    // if this Run message is meant for us.
    if (!name.isEmpty() && name != scriptRef.Get().ref)
        return; // Not our RunScript invokation - ignore it.

    if (!scriptInstance_)
    {
        LogError("Run: No script instance set");
        return;
    }

    scriptInstance_->Run();
}

/// Invoked on the Entity Action UnloadScript(scriptName).
void EC_Script::Unload(const QString &name)
{
    if (!name.isEmpty() && name != scriptRef.Get().ref)
        return; // Not our UnloadScript invokation - ignore it.

    if (!scriptInstance_)
    {
        LogError("Unload: Cannot perform, no script instance set");
        return;
    }

    scriptInstance_->Unload();
}

EC_Script::EC_Script(IModule *module):
    IComponent(module->GetFramework()),
    scriptRef(this, "Script ref"),
    type(this, "Type"),
    runOnLoad(this, "Run on load", false),
    scriptInstance_(0)
{
    static AttributeMetadata scriptRefData;
    AttributeMetadata::ButtonInfoList scriptRefButtons;
    scriptRefButtons.push_back(AttributeMetadata::ButtonInfo("runScriptButton", "P", "Run"));
    scriptRefButtons.push_back(AttributeMetadata::ButtonInfo("stopScriptButton", "S", "Unload"));
    scriptRefData.buttons = scriptRefButtons;
    scriptRef.SetMetadata(&scriptRefData);

    connect(this, SIGNAL(OnAttributeChanged(IAttribute*, AttributeChange::Type)),
        SLOT(HandleAttributeChanged(IAttribute*, AttributeChange::Type)));
    connect(this, SIGNAL(ParentEntitySet()), SLOT(RegisterActions()));

    scriptAsset = boost::shared_ptr<AssetRefListener>(new AssetRefListener);
    connect(scriptAsset.get(), SIGNAL(Loaded(AssetPtr)), this, SLOT(ScriptAssetLoaded(AssetPtr)));
}

void EC_Script::HandleAttributeChanged(IAttribute* attribute, AttributeChange::Type change)
{
    if (attribute == &scriptRef)
    {
        if (!scriptRef.Get().ref.isEmpty())
            scriptAsset->HandleAssetRefChange(attribute);
        else // If the script ref is empty we need to unload script instance.
            SetScriptInstance(0);
    }
}

void EC_Script::ScriptAssetLoaded(AssetPtr asset_)
{
    ScriptAssetPtr asset = boost::dynamic_pointer_cast<ScriptAsset>(asset_);
    if (!asset)
    {
        LogError("EC_Script::ScriptAssetLoaded: Loaded asset of type other than ScriptAsset!");
        return;
    }

    emit ScriptAssetChanged(asset);
}

void EC_Script::RegisterActions()
{
    Scene::Entity *entity = GetParentEntity();
    assert(entity);
    if (entity)
    {
        entity->ConnectAction("RunScript", this, SLOT(Run(const QString &)));
        entity->ConnectAction("UnloadScript", this, SLOT(Unload(const QString &)));
    }
}


// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "EC_Script.h"
#include "IScriptInstance.h"
#include "ScriptAsset.h"
#include "AssetAPI.h"
#include "Framework.h"
#include "IAttribute.h"
#include "AttributeMetadata.h"
#include "IAssetTransfer.h"
#include "Entity.h"
#include "AssetRefListener.h"
#include "LoggingFunctions.h"

#include "MemoryLeakCheck.h"

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
    if (!name.isEmpty() && name != Name())
        return; // Not our RunScript invocation - ignore it.

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
    if (!name.isEmpty() && name != Name())
        return; // Not our RunScript invocation - ignore it.

    if (!scriptInstance_)
    {
        LogError("Unload: Cannot perform, no script instance set");
        return;
    }

    scriptInstance_->Unload();
}

EC_Script::EC_Script(Scene* scene):
    IComponent(scene),
    scriptRef(this, "Script ref", AssetReferenceList("Script")),
    runOnLoad(this, "Run on load", false),
    runMode(this, "Run mode", 0),
    applicationName(this, "Script application name"),
    className(this, "Script class name"),
    scriptInstance_(0)
{
    static AttributeMetadata scriptRefData;
    AttributeMetadata::ButtonInfoList scriptRefButtons;
    scriptRefButtons.push_back(AttributeMetadata::ButtonInfo("runScriptButton", "P", "Run"));
    scriptRefButtons.push_back(AttributeMetadata::ButtonInfo("stopScriptButton", "S", "Unload"));
    scriptRefData.buttons = scriptRefButtons;
    scriptRefData.elementType = "assetreference";
    scriptRef.SetMetadata(&scriptRefData);

    connect(this, SIGNAL(AttributeChanged(IAttribute*, AttributeChange::Type)),
        SLOT(HandleAttributeChanged(IAttribute*, AttributeChange::Type)));
    connect(this, SIGNAL(ParentEntitySet()), SLOT(RegisterActions()));
}

void EC_Script::HandleAttributeChanged(IAttribute* attribute, AttributeChange::Type change)
{
    AssetAPI* assetAPI = framework->Asset();
    
    if (attribute == &scriptRef)
    {
        AssetReferenceList scripts = scriptRef.Get();
        // Make sure that the asset ref list type stays intact.
        scripts.type = "Scripts";
        scriptRef.Set(scripts, AttributeChange::Disconnected);

        // Purge empty script refs
        scripts.RemoveEmpty();

        // Reallocate the number of asset ref listeners.
        while(scriptAssets.size() > (size_t)scripts.Size())
            scriptAssets.pop_back();
        while(scriptAssets.size() < (size_t)scripts.Size())
            scriptAssets.push_back(boost::shared_ptr<AssetRefListener>(new AssetRefListener));

        if (scripts.Size() > 0)
        {
            QString refContext;
            
            for(int i = 0; i < scripts.Size(); ++i)
            {
                // The first script ref must be resolvable without context. Then, for each subsequent asset, the previous will be used as a reference
                QString resolvedRef = assetAPI->ResolveAssetRef(refContext, scripts[i].ref);
                refContext = resolvedRef;
                
                connect(scriptAssets[i].get(), SIGNAL(Loaded(AssetPtr)), this, SLOT(OnScriptAssetLoaded(AssetPtr)), Qt::UniqueConnection);
                scriptAssets[i]->HandleAssetRefChange(assetAPI, resolvedRef);
            }
        }
        else // If there are no non-empty script refs, we unload the script instance.
            SetScriptInstance(0);
    }
    else if (attribute == &applicationName)
    {
        emit ApplicationNameChanged(applicationName.Get());
    }
    else if (attribute == &className)
    {
        emit ClassNameChanged(className.Get());
    }
}

void EC_Script::OnScriptAssetLoaded(AssetPtr asset_)
{
    // If all asset ref listeners have valid, loaded script assets, it's time to fire up the script engine
    std::vector<ScriptAssetPtr> loadedScriptAssets;
    for (unsigned i = 0; i < scriptAssets.size(); ++i)
    {
        if (scriptAssets[i]->Asset())
        {
            ScriptAssetPtr asset = boost::dynamic_pointer_cast<ScriptAsset>(scriptAssets[i]->Asset());
            if (!asset)
            {
                LogError("EC_Script::ScriptAssetLoaded: Loaded asset of type other than ScriptAsset!");
                return;
            }
            if (asset->IsLoaded())
                loadedScriptAssets.push_back(asset);
        }
    }
    
    if (loadedScriptAssets.size() == scriptAssets.size())
        emit ScriptAssetsChanged(loadedScriptAssets);
}

void EC_Script::RegisterActions()
{
    Entity *entity = ParentEntity();
    assert(entity);
    if (entity)
    {
        entity->ConnectAction("RunScript", this, SLOT(Run(const QString &)));
        entity->ConnectAction("UnloadScript", this, SLOT(Unload(const QString &)));
    }
}


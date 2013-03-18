// For conditions of distribution and use, see copyright notice in LICENSE

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
    // If we have a classname, empty it to trigger deletion of the script object
    if (!className.Get().trimmed().isEmpty())
        className.Set("", AttributeChange::LocalOnly);
    
    if (scriptInstance_)
        scriptInstance_->Unload();
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

void EC_Script::SetScriptApplication(EC_Script* app)
{
    if (app)
        scriptApplication_ = app->shared_from_this();
    else
        scriptApplication_.reset();
}

EC_Script* EC_Script::ScriptApplication() const
{
    return dynamic_cast<EC_Script*>(scriptApplication_.lock().get());
}

bool EC_Script::ShouldRun() const
{
    int mode = runMode.Get();
    if (mode == RM_Both)
        return true;
    if (mode == RM_Client && isClient_)
        return true;
    if (mode == RM_Server && isServer_)
        return true;
    return false;
}

void EC_Script::SetIsClientIsServer(bool isClient, bool isServer)
{
    isClient_ = isClient;
    isServer_ = isServer;
}

void EC_Script::Run(const QString &name)
{
    if (!ShouldRun())
    {
        LogWarning("Run explicitly called, but RunMode does not match");
        return;
    }
    
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
    runMode(this, "Run mode", RM_Both),
    applicationName(this, "Script application name"),
    className(this, "Script class name"),
    scriptInstance_(0),
    isClient_(false),
    isServer_(false)
{
    static AttributeMetadata scriptRefData;
    static AttributeMetadata runModeData;
    static bool metadataInitialized = false;
    if (!metadataInitialized)
    {
        AttributeMetadata::ButtonInfoList scriptRefButtons;
        scriptRefButtons.push_back(AttributeMetadata::ButtonInfo("runScriptButton", "P", "Run"));
        scriptRefButtons.push_back(AttributeMetadata::ButtonInfo("stopScriptButton", "S", "Unload"));
        scriptRefData.buttons = scriptRefButtons;
        scriptRefData.elementType = "assetreference";
        runModeData.enums[RM_Both] = "Both";
        runModeData.enums[RM_Client] = "Client";
        runModeData.enums[RM_Server] = "Server";
        metadataInitialized = true;
    }
    scriptRef.SetMetadata(&scriptRefData);
    runMode.SetMetadata(&runModeData);

    connect(this, SIGNAL(AttributeChanged(IAttribute*, AttributeChange::Type)),
        SLOT(HandleAttributeChanged(IAttribute*, AttributeChange::Type)));
    connect(this, SIGNAL(ParentEntitySet()), SLOT(RegisterActions()));
}

void EC_Script::HandleAttributeChanged(IAttribute* attribute, AttributeChange::Type /*change*/)
{
    AssetAPI* assetAPI = framework->Asset();
    
    if (attribute == &scriptRef)
    {
        // Do not even fetch the assets if we should not run
        if (!ShouldRun())
        {
            scriptAssets.clear();
            return;
        }
        
        AssetReferenceList scripts = scriptRef.Get();

        // Purge empty script refs
        scripts.RemoveEmpty();

        // Reallocate the number of asset ref listeners.
        while(scriptAssets.size() > (size_t)scripts.Size())
            scriptAssets.pop_back();
        while(scriptAssets.size() < (size_t)scripts.Size())
            scriptAssets.push_back(shared_ptr<AssetRefListener>(new AssetRefListener));

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
    else if (attribute == &runMode)
    {
        // If we had not loaded script assets previously because of runmode not allowing, load them now
        if (ShouldRun())
        {
            if (scriptAssets.empty())
                HandleAttributeChanged(&scriptRef, AttributeChange::Default);
        }
        else // If runmode is changed and shouldn't run, unload script assets and script instance 
        {
            scriptAssets.clear();
            SetScriptInstance(0);
        }
    }
    else if (attribute == &runOnLoad)
    {
        // If RunOnLoad changes, is true, and we don't have a script instance yet, emit ScriptAssetsChanged to start up the script.
        if (runOnLoad.Get() && scriptAssets.size() && (!scriptInstance_ || !scriptInstance_->IsEvaluated()))
            OnScriptAssetLoaded(AssetPtr()); // The asset ptr can be null, it is not used.
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
            ScriptAssetPtr asset = dynamic_pointer_cast<ScriptAsset>(scriptAssets[i]->Asset());
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


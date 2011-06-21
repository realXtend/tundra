// For conditions of distribution and use, see copyright notice in license.txt

#pragma once

#include "IComponent.h"
#include "AssetReference.h"
#include "AssetFwd.h"

class IScriptInstance;

class ScriptAsset;
typedef boost::shared_ptr<ScriptAsset> ScriptAssetPtr;

/// Provides mechanism for adding scripts to entities.
/**
<table class="header">
<tr>
<td>
<h2>Script</h2>
Provides mechanism for adding scripts to entities.
Registered by PythonScript::PythonScriptModule and/or JavascriptModule.

<b>Attributes</b>:
<ul>
<li>AssetReferenceList: scriptRef
<div>The script assets that will be loaded. If empty, no script engine will be created</div> 
<li>bool: runOnLoad
<div>Is the script engine run as soon as the script asset(s) are set/loaded.</div>
<li>int: runMode
<div>Whether to run on client, server or both.</div>
<li>QString: applicationName
<div>Name for the script application. Script components without an own engine refer to the application by name.</div>
<li>QString: className
<div>The script class to instantiate from within an existing application. Syntax: applicationName.className</div>
</ul>

<b>Exposes the following scriptable functions:</b>
<ul>
<li>...
</ul>

<b>Reacts on the following actions:</b>
<ul>
<li> "RunScript": Runs the script. Usage: RunScript [componentname]
<li> "UnloadScript": Stops and unloads the script. Usage: UnloadScript [componentname]
</ul>
</td>
</tr>

Does not emit any actions.

<b>Doesn't depend on any other entity-component</b>.
</table>
*/
class EC_Script: public IComponent
{
    Q_OBJECT

public:
    /// Do not directly allocate new components using operator new, but use the factory-based SceneAPI::CreateComponent functions instead.
    explicit EC_Script(Scene* scene);

    ~EC_Script();

    /// Script asset reference
    Q_PROPERTY(AssetReferenceList scriptRef READ getscriptRef WRITE setscriptRef);
    DEFINE_QPROPERTY_ATTRIBUTE(AssetReferenceList, scriptRef);

    /// Is the script engine run as soon as the script asset(s) are set/loaded.
    Q_PROPERTY(bool runOnLoad READ getrunOnLoad WRITE setrunOnLoad);
    DEFINE_QPROPERTY_ATTRIBUTE(bool, runOnLoad);

    /// Whether to run on client, server or both.</div>
    Q_PROPERTY(int runMode READ getrunMode WRITE setrunMode);
    DEFINE_QPROPERTY_ATTRIBUTE(int, runMode);
    
    /// Name for the script application. Script components without an own engine refer to the application by name.
    Q_PROPERTY(QString applicationName READ getapplicationName WRITE setapplicationName);
    DEFINE_QPROPERTY_ATTRIBUTE(QString, applicationName);
    
    /// The script class to instantiate from within an existing application. Syntax: applicationName.className
    Q_PROPERTY(QString className READ getclassName WRITE setclassName);
    DEFINE_QPROPERTY_ATTRIBUTE(QString, className);
    
    /// Sets new script instance.
    /** Unloads and deletes possible already existing script instance.
        @param instance Script instance.
        @note Takes ownership of the script instace.
    */
    void SetScriptInstance(IScriptInstance *instance);

    /// Returns the current script instance.
    IScriptInstance *GetScriptInstance() const { return scriptInstance_; }

    COMPONENT_NAME("EC_Script", 5)
public slots:

    /// Runs the script instance.
    /** @param name Name of the script component, optional. The script will be run only if the component name matches.
    */
    void Run(const QString& name = QString());

    /// Stops and unloads the script.
    /** @param name Name(s) of the script(s), optional. The script is unloaded only if the script name matches.
    */
    void Unload(const QString& name = QString());

signals:
    /// Emitted when changed script assets are ready to run
    void ScriptAssetsChanged(const std::vector<ScriptAssetPtr>& newScripts);

    /// Emitted when the script application name has changed
    void ApplicationNameChanged(const QString& appName);

    /// Emitted when the script class name has changed
    void ClassNameChanged(const QString& className);

private slots:
    /// Handles logic regarding attribute changes of this EC.
    /** @param attribute Attribute that changed.
        @param change Change type.
    */
    void HandleAttributeChanged(IAttribute* attribute, AttributeChange::Type change);

    /// Called when a script asset has been loaded.
    void OnScriptAssetLoaded(AssetPtr asset);

    /// Registers the actions this component provides when parent entity is set.
    void RegisterActions();

private:
    /// Handles the downloading of script assets.
    std::vector<boost::shared_ptr<AssetRefListener> > scriptAssets;

    /// Script instance.
    IScriptInstance *scriptInstance_;
    
public:
    /// The parent script application, if an object has been instantiated from inside it
    ComponentWeakPtr scriptApplication_;
};


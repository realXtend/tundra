// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "IComponent.h"
#include "AssetReference.h"
#include "AssetFwd.h"

class IScriptInstance;
class ScriptAsset;
typedef shared_ptr<ScriptAsset> ScriptAssetPtr;

/// Provides mechanism for adding scripts to entities.
/** <table class="header">
    <tr>
    <td>
    <h2>Script</h2>
    Provides mechanism for adding scripts to entities.
    Registered by JavascriptModule and/or PythonScript::PythonScriptModule.

    When EC_Script has scriptRef(s) set, it will load and run the script assets in an own script engine. Optionally,
    it can define the name of the script application.

    EC_Script's can also create script objects within an existing script application. In this case, the scriptRef
    is left empty, and instead the className attribute tells from which script application to instantiate a specific class.
    The syntax for className attribute is ApplicationName.ClassName . The EC_Script with the matching application will be
    looked up, and the constructor function ClassName will be called, with the entity and component as parameters.
    If the classname changes, the previously created script object will be automatically destroyed. Script objects
    may optionally define a destructor called OnScriptObjectDestroyed

    <b>Attributes</b>:
    <ul>
    <li>AssetReferenceList: scriptRef
    <div> @copydoc scriptRef </div>
    <li>bool: runOnLoad
    <div> @copydoc runOnLoad </div>
    <li>enum: runMode
    <div> @copydoc runMode </div>
    <li>QString: applicationName
    <div>@copydoc applicationName </div>
    <li>QString: className
    <div>@copydoc className </div>
    </ul>

    <b>Exposes the following scriptable functions:</b>
    <ul>
    <li>"Run": @copydoc Run
    <li>"Unload": @copydoc Unload
    <li>"ScriptObjectKey": @copydoc ScriptObjectKey
    <li>"ShouldRun": @copydoc ShouldRun
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
    </table> */
class EC_Script: public IComponent
{
    Q_OBJECT
    COMPONENT_NAME("EC_Script", 5)

public:
    /// Run mode enumeration
    enum RunMode
    {
        RM_Both = 0,
        RM_Client,
        RM_Server
    };
    
    /// Do not directly allocate new components using operator new, but use the factory-based SceneAPI::CreateComponent functions instead.
    explicit EC_Script(Scene* scene);

    ~EC_Script();

    /// The script assets that will be loaded. If empty, no script engine will be created
    Q_PROPERTY(AssetReferenceList scriptRef READ getscriptRef WRITE setscriptRef);
    DEFINE_QPROPERTY_ATTRIBUTE(AssetReferenceList, scriptRef);

    /// Is the script engine run as soon as the script asset(s) are set/loaded
    Q_PROPERTY(bool runOnLoad READ getrunOnLoad WRITE setrunOnLoad);
    DEFINE_QPROPERTY_ATTRIBUTE(bool, runOnLoad);

    /// Whether to run on client, server or both
    Q_PROPERTY(int runMode READ getrunMode WRITE setrunMode);
    DEFINE_QPROPERTY_ATTRIBUTE(int, runMode);
    
    /// Name for the script application
    Q_PROPERTY(QString applicationName READ getapplicationName WRITE setapplicationName);
    DEFINE_QPROPERTY_ATTRIBUTE(QString, applicationName);
    
    /// The script class to instantiate from within an existing application. Syntax: applicationName.className
    Q_PROPERTY(QString className READ getclassName WRITE setclassName);
    DEFINE_QPROPERTY_ATTRIBUTE(QString, className);
    
    /// Sets new script instance.
    /** Unloads and deletes possible already existing script instance.
        @param instance Script instance.
        @note Takes ownership of the script instace. */
    void SetScriptInstance(IScriptInstance *instance);

    /// Returns the current script instance.
    IScriptInstance *ScriptInstance() const { return scriptInstance_; }

    /// Set the application script component this script component has a script object created from
    void SetScriptApplication(EC_Script* app);
    
    /// Return the script application component, if it (still) exists
    EC_Script* ScriptApplication() const;

    /// Set the IsServer and IsClient flags. Called by the parent scripting system, which has this knowledge
    void SetIsClientIsServer(bool isClient, bool isServer);

public slots:
    /// Runs the script instance.
    /** @param name Name of the script component, optional. The script will be run only if the component name matches.*/
    void Run(const QString& name = QString());

    /// Stops and unloads the script.
    /** @param name Name(s) of the script(s), optional. The script is unloaded only if the script name matches. */
    void Unload(const QString& name = QString());

    /// Return the key for accessing the script engine's scriptObjects property map
    QString ScriptObjectKey() const { return QString::number((u64)this); }

    /// Check whether the script should run
    bool ShouldRun() const;

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
        @param change Change type. */
    void HandleAttributeChanged(IAttribute* attribute, AttributeChange::Type change);

    /// Called when a script asset has been loaded.
    void OnScriptAssetLoaded(AssetPtr asset);

    /// Registers the actions this component provides when parent entity is set.
    void RegisterActions();

private:
    /// Handles the downloading of script assets.
    std::vector<AssetRefListenerPtr> scriptAssets;

    /// Script instance.
    IScriptInstance *scriptInstance_;
    
    /// The parent script application, if an object has been instantiated from inside it
    ComponentWeakPtr scriptApplication_;
    
    /// IsClient flag, for checking run mode
    bool isClient_;
    /// IsServer flag, for checking run mode
    bool isServer_;
};

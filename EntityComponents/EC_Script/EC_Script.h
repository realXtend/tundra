// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_EC_Script_EC_Script_h
#define incl_EC_Script_EC_Script_h

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
<li>AssetReference: scriptRef
<div>Reference the the script asset.</div> 
<li>QString: type
<div>Type of the script as string (js/py).</div> 
<li>bool: runOnLoad
<div>Is the script run as soon as the script reference is set/loaded.</div> 
</ul>

<b>Exposes the following scriptable functions:</b>
<ul>
<li>...
</ul>

<b>Reacts on the following actions:</b>
<ul>
<li> "RunScript": Runs the script. Usage: RunScript [filename]
<li> "UnloadScript": Stops and unloads the script. Usage: UnloadScript [filename]
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
    explicit EC_Script(Framework *fw);

    ~EC_Script();

    /// Type of the script as string (js/py)
    Q_PROPERTY(QString type READ gettype WRITE settype);
    DEFINE_QPROPERTY_ATTRIBUTE(QString, type);

    /// Is the script run as soon as the script reference is set/loaded.
    Q_PROPERTY(bool runOnLoad READ getrunOnLoad WRITE setrunOnLoad);
    DEFINE_QPROPERTY_ATTRIBUTE(bool, runOnLoad);

    /// Script asset reference
    Q_PROPERTY(AssetReference scriptRef READ getscriptRef WRITE setscriptRef);
    DEFINE_QPROPERTY_ATTRIBUTE(AssetReference, scriptRef);

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
    /** @param name Name of the script, optional. The script is ran only if the script name matches.
    */
    void Run(const QString &name = QString());

    /// Stops and unloads the script.
    /** @param name Name of the script, optional. The script is unloaded only if the script name matches.
    */
    void Unload(const QString &name = QString());

signals:
    /// Emitted when script reference changes.
    void ScriptAssetChanged(ScriptAssetPtr newScript);

private slots:
    /// Handles logic regarding attribute changes of this EC.
    /** @param attribute Attribute that changed.
        @param change Change type.
    */
    void HandleAttributeChanged(IAttribute* attribute, AttributeChange::Type change);

    /// Called when a new script asset has been loaded.
    void ScriptAssetLoaded(AssetPtr asset);

    /// Registers the actions this component provides when parent entity is set.
    void RegisterActions();

private:
    /// Handles the downloading of script assets.
    boost::shared_ptr<AssetRefListener> scriptAsset;

    /// Script instance.
    IScriptInstance *scriptInstance_;

    /// Last reference. We're keeping track so that we don't send multiple changed signals.
    QString lastRef_;
};

#endif

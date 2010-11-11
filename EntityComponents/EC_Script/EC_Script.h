// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_EC_Script_EC_Script_h
#define incl_EC_Script_EC_Script_h

#include "IComponent.h"
#include "Declare_EC.h"
#include "AssetReference.h"

class IScriptInstance;

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
<li> "StopScript": Stops the script execution. Usage: StopScript [filename]
<li> "ReloadScript": Reloads the script. Runs the script if runOnLoad is true. Usage: ReloadScript [filename]
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
    DECLARE_EC(EC_Script)

public:
    /// Destructor.
    ~EC_Script();

    /// IComponent override. This component is serializable.
    virtual bool IsSerializable() const { return true; }

    /// Type of the script as string (js/py)
    Q_PROPERTY(QString type READ gettype WRITE settype);
    DEFINE_QPROPERTY_ATTRIBUTE(QString, type);

    /// Is the script run as soon as the script reference is set/loaded.
    Q_PROPERTY(bool runOnLoad READ getrunOnLoad WRITE setrunOnLoad);
    DEFINE_QPROPERTY_ATTRIBUTE(bool, runOnLoad);

    //! Script asset reference
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

public slots:
    /// Runs the script instance.
    /** @param name Name of the script, optional. The script is ran only if the script name matches.
    */
    void Run(const QString &name = QString());

    /// Stops the script instance.
    /** @param name Name of the script, optional. The script is stopped only if the script name matches.
    */
    void Stop(const QString &name = QString());

    /// Stops the script instance. Runs the script if runOnLoad is true
    /** @param name Name of the script, optional. The script is reloaded only if the script name matches.
    */
    void Reload(const QString &name = QString());

signals:
    /// Emitted when script reference changes.
    /** @newRef New script reference.
    */
    void ScriptRefChanged(const QString &newRef);

private slots:
    /// Handles logic regarding attribute changes of this EC.
    /** @param attribute Attribute that changed.
        @param change Change type.
    */
    void HandleAttributeChanged(IAttribute* attribute, AttributeChange::Type change);

    /// Registers the actions this component provides when parent entity is set.
    void RegisterActions();

private:
    /// Constuctor.
    /** @param module Declaring module.
    */
    explicit EC_Script(IModule *module);

    /// Script instance.
    IScriptInstance *scriptInstance_;

    /// Last reference. We're keeping track so that we don't send multiple changed signals.
    QString lastRef_;
};

#endif

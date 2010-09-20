// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_EC_Script_EC_Script_h
#define incl_EC_Script_EC_Script_h

#include "ComponentInterface.h"
#include "Declare_EC.h"



class IScriptInstance;

/**
<table class="header">
<tr>
<td>
<h2>Script</h2>
Registered by PythonScript::PythonScriptModule.

<b>Attributes</b>:
<ul>
<li>QString: scriptRef
<div></div> 
<li>QString: type
<div></div> 
</ul>

<b>Exposes the following scriptable functions:</b>
<ul>
<li>...
</ul>

<b>Reacts on the following actions:</b>
<ul>
<li>...
</ul>
</td>
</tr>

Does not emit any actions.

<b>Doesn't depend on any entity Component</b>.
</table>
*/
class EC_Script: public Foundation::ComponentInterface
{


    Q_OBJECT
    DECLARE_EC(EC_Script)

public:
    /// Destructor.
    ~EC_Script();

    /// ComponentInterface override. This component is serializable.
    virtual bool IsSerializable() const { return true; }

    /// Reference to a script file.
    Attribute<QString> scriptRef;

    /// Type of the script as string (js/py)
    Attribute<QString> type;

    /** Sets new script instance. Unloads and deletes possible already existing script instance.
        @param instance Script instance.
    */
    void SetScriptInstance(IScriptInstance *instance);

    /// Returns the current script instance.
    IScriptInstance *GetScriptInstance() const { return scriptInstance_; }

signals:
    /** Emitted when script reference changes.
        @newRef New script reference.
    */
    void ScriptRefChanged(const QString &newRef);

private slots:
    /**
        @param attribute Attribute that changed.
        @param change Change type.
    */
    void HandleAttributeChanged(AttributeInterface* attribute, AttributeChange::Type change);

private:
    /** Constuctor.
        @param module Declaring module.
    */
    explicit EC_Script(Foundation::ModuleInterface *module);

    /// Script instance.
    IScriptInstance *scriptInstance_;
};

#endif

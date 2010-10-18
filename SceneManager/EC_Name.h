/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   EC_Name.h
 *  @brief  EC_Name provides network-synchronizable means of identification for entities in addition
 *          to the plain ID number. This EC is not present by default for entities.
 */

#include "IComponent.h"
#include "IModule.h"
#include "Declare_EC.h"


/// Provides network-synchronizable means of identification for entities in addition to the plain ID number
/**
<table class="header">
<tr>
<td>
<h2>Name</h2>
Provides network-synchronizable means of identification for entities in addition to the plain ID number. This EC is not present by default for entities.

Registered by RexLogic::RexLogicModule.

<b>Attributes</b>:
<ul>
<li> QString: Name
<div>Name</div> 
<li> QString: description
<div>Description</div> 
<li> bool: userDefined
<div>Boolean which indicates that the current name value is defined by the user and should not be set programmatically.</div> 
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

</table>
*/
class EC_Name : public IComponent
{
    DECLARE_EC(EC_Name);
    Q_OBJECT

public:
    /// Desctructor.
    ~EC_Name() {}

    /// IComponent override.
    /// This component is serializable.
    virtual bool IsSerializable() const { return true; }

    /// Name.
    Attribute<QString> name;

    /// Description.
    Attribute<QString> description;

    /// Boolean which indicates that the current name value is defined by the user and should not be set programmatically.
    Attribute<bool> userDefined;

private:
    /// Constructor. Sets name and description to empty strings.
    EC_Name(IModule *module) :
        IComponent(module->GetFramework()),
        name(this, "name", ""),
        description(this, "description", ""),
        userDefined(this, "user-defined", false)
    {
        SetNetworkSyncEnabled(true);
    }
};

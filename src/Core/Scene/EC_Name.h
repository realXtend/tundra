/**
 *  For conditions of distribution and use, see copyright notice in LICENSE
 *
 *  @file   EC_Name.h
 *  @brief  EC_Name provides network-synchronizable means of identification for entities in addition
 *          to the plain ID number. This EC is not present by default for entities.
 */

#pragma once

#include "IComponent.h"

/// Allows identifying the Entity using a name string in addition to the Entity ID number.
/**
<table class="header">
<tr>
<td>
<h2>Name</h2>
Provides network-synchronizable means of identification for entities in addition to the plain ID number.
This EC is not present by default for entities.

Registered by TundraLogicModule.

<b>Attributes</b>:
<ul>
<li> QString: Name
<div>Name</div> 
<li> QString: description
<div>Description</div> 
</ul>

Does not emit any actions.

</table>
*/
class EC_Name : public IComponent
{
    Q_OBJECT
    COMPONENT_NAME("EC_Name", 26)

public:
    /// Do not directly allocate new components using operator new, but use the factory-based SceneAPI::CreateComponent functions instead.
    explicit EC_Name(Scene* scene) :
        IComponent(scene),
        name(this, "name", ""),
        description(this, "description", "")
    {}

    ~EC_Name() {}

    /// Name
    DEFINE_QPROPERTY_ATTRIBUTE(QString, name);
    Q_PROPERTY(QString name READ getname WRITE setname); 

    /// Description.
    DEFINE_QPROPERTY_ATTRIBUTE(QString, description);
    Q_PROPERTY(QString description READ getdescription WRITE setdescription); 
};

/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   EC_Selected.h
 *  @brief  EC_Selected denotes its entity is selected
 *  @author Nathan Letwory | http://www.letworyinteractive.com
 */
 
#pragma once

#include "IComponent.h"
#include "IAttribute.h"
#include "Declare_EC.h"

/// Selection of an entity
/**
<table class="header">
<tr>
<td>
<h2>Selected</h2>
Denote that entity is selected

Registered by RexLogic::RexLogicModule.

Doesn't have any attributes.

Doesn't expose scriptable functions.

Doesn't react on any actions.

Does not emit any actions.

</table>
*/
class EC_Selected : public IComponent
{
    Q_OBJECT
    DECLARE_EC(EC_Selected);

public:
    /// Destructor.
    ~EC_Selected();

public slots:
    /// IComponent override. Returns true.
    virtual bool IsTemporary() const { return true; }

private:
    /// Constuctor.
    /// @param module Owner module.
    explicit EC_Selected(IModule *module);
};


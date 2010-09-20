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

/// EC_Name provides network-synchronizable means of identification for entities in addition to the plain ID number.
class EC_Name : public IComponent
{
    DECLARE_EC(EC_Name);
    Q_OBJECT

public:
    /// Desctructor.
    ~EC_Name() {}

    /// ComponentInterface override.
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
    }
};

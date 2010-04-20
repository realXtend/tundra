// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_EC_OpenSimPresence_h
#define incl_EC_OpenSimPresence_h

#include "ComponentInterface.h"
#include "RexUUID.h"
#include "Declare_EC.h"

//! This component is present on all agents when connected to an OpenSim world. 
class EC_OpenSimPresence : public Foundation::ComponentInterface
{
    Q_OBJECT

    DECLARE_EC(EC_OpenSimPresence);

public:
    virtual ~EC_OpenSimPresence();

    //! set first name
    void SetFirstName(const std::string &name);

    //! get first name
    std::string GetFirstName() const;

    //! set last name
    void SetLastName(const std::string &name);

    //! get last name
    std::string GetLastName() const;

    //! get full name
    std::string GetFullName() const;

    // !ID related
    uint64_t regionHandle;
    int32_t localId;
    RexUUID agentId;
    uint32_t parentId;

private:
    /// Constructor.
    /// @param module Owner module.
    EC_OpenSimPresence(Foundation::ModuleInterface* module);

    //! first name of avatar
    std::string first_name_;

    //! last name of avatar
    std::string last_name_;
};

#endif

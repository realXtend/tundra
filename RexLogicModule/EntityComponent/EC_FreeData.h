// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_EC_FreeData_h
#define incl_EC_FreeData_h

#include "ComponentInterface.h"
#include "RexLogicModuleApi.h"
#include "Declare_EC.h"

namespace RexLogic
{
    /// An entity component that attaches to an entity a generic string of data, for quickly doing custom data extension/storage.
    /// \todo This component is not useful unless there can be several instances of it on a single entity, or unless we make it store a dictionary
    ///       of string-value pairs.
    class EC_FreeData : public Foundation::ComponentInterface
    {
        Q_OBJECT
            
        DECLARE_EC(EC_FreeData);
       
    public:
        virtual ~EC_FreeData();

        std::string FreeData;
        
    private:
        EC_FreeData(Foundation::ModuleInterface* module);

    };
}

#endif
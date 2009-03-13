// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_RexLogic_System_h
#define incl_RexLogic_System_h

#include "ModuleInterface.h"
#include "WorldLogic.h"

//! interface for modules
class RexLogic : public Foundation::ModuleInterface_Impl
{
public:
    RexLogic();
    virtual ~RexLogic();

    virtual void Load();
    virtual void Unload();
    virtual void Initialize(Foundation::Framework *framework);
    virtual void Uninitialize(Foundation::Framework *framework);

    virtual void Update();
    
    MODULE_LOGGING_FUNCTIONS;

    //! returns name of this module. Needed for logging.
    static const std::string &NameStatic() { return Foundation::Module::NameFromType(type_static_); }

    static const Foundation::Module::Type type_static_ = Foundation::Module::MT_WorldLogic;

private:
    Foundation::Framework *framework_;
    
    WorldLogic *world_logic_;
    
};

#endif

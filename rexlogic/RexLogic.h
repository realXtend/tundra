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

    virtual void load();
    virtual void unload();
    virtual void initialize(Foundation::Framework *framework);
    virtual void uninitialize(Foundation::Framework *framework);

    virtual void update();

private:
    Foundation::Framework *mFramework;
    
    WorldLogic *mWorldLogic;
    
};

#endif

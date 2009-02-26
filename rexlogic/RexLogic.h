#ifndef __incl_RexLogic_System_h__
#define __incl_RexLogic_System_h__

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

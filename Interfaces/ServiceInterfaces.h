// For conditions of distribution and use, see copyright notice in license.txt

#ifndef __incl_Interfaces_ServiceInterfaces_h__
#define __incl_Interfaces_ServiceInterfaces_h__

// When adding new service type, add the interface include here too
#include "PhysicsServiceInterface.h"
#include "RenderServiceInterface.h"
#include "ScriptServiceInterface.h"
#include "SoundServiceInterface.h"
#include "GuiServiceInterface.h"
#include "InputServiceInterface.h"
#include "WorldLogicInterface.h" //! \todo should this be here?
#include "TestServiceInterface.h"

namespace Foundation
{
    namespace Service
    {
        enum Type { ST_Renderer, ST_Physics, ST_Gui, ST_Scripting, ST_Test };
    }
}

#endif


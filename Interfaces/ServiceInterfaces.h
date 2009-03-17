// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Interfaces_ServiceInterfaces_h
#define incl_Interfaces_ServiceInterfaces_h

// When adding new service type, add the interface include here too
#include "PhysicsServiceInterface.h"
#include "RenderServiceInterface.h"
#include "ScriptServiceInterface.h"
#include "SoundServiceInterface.h"
#include "GuiServiceInterface.h"
#include "InputServiceInterface.h"
#include "WorldLogicInterface.h" //! \todo should this be here?
#include "TestServiceInterface.h"
#include "SceneManagerServiceInterface.h"
#include "ConsoleServiceInterface.h"

namespace Foundation
{
    namespace Service
    {
        enum Type {ST_SceneManager = 0, ST_Renderer, ST_Physics, ST_Gui, ST_Scripting, ST_Test, ST_Console, ST_Unknown };
    }
}

#endif


// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Interfaces_ServiceInterfaces_h
#define incl_Interfaces_ServiceInterfaces_h

// When adding new service type, add the interface include here too
#include "RenderServiceInterface.h"
#include "ScriptServiceInterface.h"
#include "SoundServiceInterface.h"
#include "WorldLogicInterface.h"
#include "ConsoleServiceInterface.h"
#include "ConsoleCommandServiceInterface.h"
#include "AssetServiceInterface.h"
#include "TextureServiceInterface.h"

namespace Foundation
{
    /*! \defgroup Services_group Services Client Interface
        \copydoc Service
    */

    //! Services offer a way for (core) modules to communicate with each other.
    /*! 
        For details, see \ref ModuleServices "Module Services".

        \ingroup Foundation_group
        \ingroup Services_group
    */
    namespace Service
    {
        //! Service types. When creating a new core service, add it here.
        /*! If you can't or don't want to alter the framework, an arbitrary
            int can be used as service type. For details, see \ref creating_services.

            \ingroup Services_group
        */
        enum Type {ST_Renderer, ST_Physics, ST_Gui, ST_Scripting, ST_Console, ST_ConsoleCommand, ST_Asset, ST_Texture, ST_Sound, ST_Input, ST_Communication, ST_Unknown };
    }
}

#endif


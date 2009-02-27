// For conditions of distribution and use, see copyright notice in license.txt

#ifndef __incl_GeometrySystem_System_h__
#define __incl_GeometrySystem_System_h__

#include "ModuleInterface.h"

namespace Foundation
{
    class Framework;
}

namespace Geometry
{
    //! interface for modules
    class GeometrySystem : public Foundation::ModuleInterface_Impl
    {
    public:
        GeometrySystem();
        virtual ~GeometrySystem();

        virtual void load();
        virtual void unload();
        virtual void initialize(Foundation::Framework *framework);
        virtual void uninitialize(Foundation::Framework *framework);
    };
}

#endif

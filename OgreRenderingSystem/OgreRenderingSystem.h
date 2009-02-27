// For conditions of distribution and use, see copyright notice in license.txt

#ifndef __incl_OgreRenderingSystem_System_h__
#define __incl_OgreRenderingSystem_System_h__

#include "ModuleInterface.h"
#include "Renderer.h"

namespace Foundation
{
    class Framework;
}

namespace OgreRenderer
{
    //! interface for modules
    class OgreRenderingSystem : public Foundation::ModuleInterface_Impl
    {
    public:
        OgreRenderingSystem();
        virtual ~OgreRenderingSystem();

        virtual void load();
        virtual void unload();
        virtual void initialize(Foundation::Framework *framework);
        virtual void uninitialize(Foundation::Framework *framework);

    private:
        OgreRenderer::RendererPtr mRenderer;
    };
}

#endif

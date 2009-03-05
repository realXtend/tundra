// For conditions of distribution and use, see copyright notice in license.txt

#ifndef __incl_OgreRenderingModule_h__
#define __incl_OgreRenderingModule_h__

#include "ModuleInterface.h"
#include "Renderer.h"

namespace Foundation
{
    class Framework;
}

namespace OgreRenderer
{
    //! interface for modules
    class OgreRenderingModule : public Foundation::ModuleInterface_Impl
    {
    public:
        OgreRenderingModule();
        virtual ~OgreRenderingModule();

        virtual void load();
        virtual void unload();
        virtual void initialize(Foundation::Framework *framework);
        virtual void uninitialize(Foundation::Framework *framework);

    private:
        OgreRenderer::RendererPtr mRenderer;
    };
}

#endif

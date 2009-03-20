// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OgreRenderingModule_h
#define incl_OgreRenderingModule_h

#include "ModuleInterface.h"
#include "Renderer.h"
#include "ConsoleCommandServiceInterface.h"

namespace Foundation
{
    class Framework;
}

namespace OgreRenderer
{
    class Renderer;
    typedef boost::shared_ptr<Renderer> RendererPtr;
    
    //! interface for modules
    class MODULE_API OgreRenderingModule : public Foundation::ModuleInterface_Impl
    {
    public:
        OgreRenderingModule();
        virtual ~OgreRenderingModule();

        virtual void Load();
        virtual void Unload();
        virtual void Initialize(Foundation::Framework *framework);
        virtual void PostInitialize(Foundation::Framework *framework);
        virtual void Uninitialize(Foundation::Framework *framework);
        virtual void Update();

        //! returns framework
        Foundation::Framework *GetFramework() { return framework_; }
        
        //! returns renderer
        RendererPtr GetRenderer() const { return renderer_; }

        //! callback for console command
        Console::CommandResult ConsoleSetViewportColor(const Core::StringVector &params);
        
        MODULE_LOGGING_FUNCTIONS;

        //! returns name of this module. Needed for logging.
        static const std::string &NameStatic() { return Foundation::Module::NameFromType(type_static_); }

        static const Foundation::Module::Type type_static_ = Foundation::Module::MT_Renderer;
    private:
        RendererPtr renderer_;
        Foundation::Framework* framework_;
    };
}

#endif

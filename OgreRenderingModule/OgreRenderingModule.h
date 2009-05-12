// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OgreRenderingModule_h
#define incl_OgreRenderingModule_h

#include "Foundation.h"
#include "ModuleInterface.h"
#include "Renderer.h"
#include "OgreModuleApi.h"

namespace Foundation
{
    class Framework;
}

namespace OgreRenderer
{
    class Renderer;
    typedef boost::shared_ptr<Renderer> RendererPtr;
    
    /** \defgroup OgreRenderingModuleClient OgreRenderingModule Client Interface
        This page lists the public interface of the OgreRenderingModule.

        For details on how to use the public interface, see \ref OgreRenderingModule "Using the Ogre renderer module"
    */

    //! A renderer module using Ogre
    class OGRE_MODULE_API OgreRenderingModule : public Foundation::ModuleInterfaceImpl
    {
    public:
        OgreRenderingModule();
        virtual ~OgreRenderingModule();

        virtual void Load();
        virtual void Unload();
        virtual void PreInitialize();
        virtual void Initialize();
        virtual void PostInitialize();
        virtual void Uninitialize();
        virtual void Update(Core::f64 frametime);
        
        virtual bool HandleEvent(
            Core::event_category_id_t category_id,
            Core::event_id_t event_id, 
            Foundation::EventDataInterface* data);

        //! returns renderer
        RendererPtr GetRenderer() const { return renderer_; }
        
        MODULE_LOGGING_FUNCTIONS;

        //! returns name of this module. Needed for logging.
        static const std::string &NameStatic() { return Foundation::Module::NameFromType(type_static_); }

        //! callback for console command
        Console::CommandResult ConsoleRequestTexture(const Core::StringVector &params);

        //! callback for console command
        Console::CommandResult ConsoleRequestMesh(const Core::StringVector &params);

        static const Foundation::Module::Type type_static_ = Foundation::Module::MT_Renderer;

    private:
        //! renderer
        RendererPtr renderer_;

        //! asset event category
        Core::event_category_id_t assetcategory_id_;

        //! resource event category
        Core::event_category_id_t resourcecategory_id_;
    };
}

#endif

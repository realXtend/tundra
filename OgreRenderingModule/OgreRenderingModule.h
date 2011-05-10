// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OgreRenderingModule_OgreRenderingModule_h
#define incl_OgreRenderingModule_OgreRenderingModule_h

#include "IModule.h"

#include "OgreModuleApi.h"

class Framework;
struct ConsoleCommandResult;

namespace OgreRenderer
{
    class Renderer;
    typedef boost::shared_ptr<Renderer> RendererPtr;
    class RendererSettings;
    typedef boost::shared_ptr<RendererSettings> RendererSettingsPtr;

    /// \bug Ogre assert fail when viewing a mesh that contains a reference to non-existing skeleton.
    
    /** \defgroup OgreRenderingModuleClient OgreRenderingModule Client Interface
        This page lists the public interface of the OgreRenderingModule.

        For details on how to use the public interface, see \ref OgreRenderingModule "Using the Ogre renderer module"
    */

    /// A renderer module using Ogre
    class OGRE_MODULE_API OgreRenderingModule : public IModule
    {
    public:
        OgreRenderingModule();
        virtual ~OgreRenderingModule();

        virtual void Load();
        virtual void PreInitialize();
        virtual void Initialize();
        virtual void PostInitialize();
        virtual void Uninitialize();
        virtual void Update(f64 frametime);

        /// returns renderer
        RendererPtr GetRenderer() const { return renderer_; }

        /// callback for console command
        ConsoleCommandResult ConsoleStats(const StringVector &params);

    private:
        /// renderer
        RendererPtr renderer_;

        /// renderer settings
        RendererSettingsPtr renderer_settings_;
    };
}

#endif

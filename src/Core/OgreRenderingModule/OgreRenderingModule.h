// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "IModule.h"
#include "OgreModuleApi.h"
#include "OgreModuleFwd.h"
#include "SceneFwd.h"
#include <OgreRenderSystem.h>

namespace OgreRenderer
{
    /** @defgroup OgreRenderingModuleClient OgreRenderingModule Client Interface
        This page lists the public interface of the OgreRenderingModule.

        For details on how to use the public interface, see \ref OgreRenderingModule "Using the Ogre renderer module"

        @bug Ogre assert fail when viewing a mesh that contains a reference to non-existing skeleton. */

    /// A renderer module using Ogre
    class OGRE_MODULE_API OgreRenderingModule : public IModule
    {
        Q_OBJECT

    public:
        OgreRenderingModule();
        virtual ~OgreRenderingModule();

        virtual void Load();
        virtual void Initialize();
        virtual void Uninitialize();

        /// Returns the renderer.
        const RendererPtr &Renderer() const { return renderer; }

        /// Ogre resource group for cached asset files.
        static std::string CACHE_RESOURCE_GROUP;

        // DEPRECATED
        /// @cond PRIVATE
        const RendererPtr &GetRenderer() const { return Renderer(); } /**< @deprecated Use Renderer() instead. @todo Remove. */
        /// @endcond

    signals:
        /// DirectX device lost signal.
        void DeviceLost();
        /// DirectX device restored signal.
        void DeviceRestored();
        /// DirectX device created signal.
        void DeviceCreated();
        /// DirectX device released signal.
        void DeviceReleased();

    public slots:
        /// Prints renderer stats to console.
        void ConsoleStats();

        /// Toggles visibility of the Ogre profiler overlay.
        /** @note Applicable only if Ogre built with profiler support. */
        void ToggleOgreProfilerOverlay();

        /// Sets attribute value for material.
        void SetMaterialAttribute(const QStringList &params);

    private slots:
        /// Creates OgreWorld for a Scene.
        void CreateOgreWorld(Scene *scene);
        /// Removes OgreWorld from a Scene.
        void RemoveOgreWorld(Scene *scene);

    protected:
        /// Emit DeviceLost signal.
        void EmitDeviceLost();
        /// Emit DeviceRestored signal.
        void EmitDeviceRestored();
        /// Emit DeviceCreated signal.
        void EmitDeviceCreated();
        /// Emit DeviceReleased signal.
        void EmitDeviceReleased();

    private:
        RendererPtr renderer;  ///< Renderer

        OgreRenderSystemListener* renderSystemListener;
        friend class OgreRenderSystemListener;
    };

    class OgreRenderSystemListener : public Ogre::RenderSystem::Listener
    {
    public:
        OgreRenderSystemListener(OgreRenderer::OgreRenderingModule* renderingModule);
        ~OgreRenderSystemListener();

        void eventOccurred(const Ogre::String& eventName, const Ogre::NameValuePairList* parameters = 0);

    private:
        OgreRenderer::OgreRenderingModule* _renderingModule;
    };
}

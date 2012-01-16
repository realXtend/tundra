// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "IModule.h"
#include "OgreModuleApi.h"
#include "OgreModuleFwd.h"

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
        RendererPtr GetRenderer() const { return renderer; }

        /// Ogre resource group for cached asset files.
        static std::string CACHE_RESOURCE_GROUP;

    public slots:
        /// Prints renderer stats to console.
        void ConsoleStats();

        /// Sets attribute value for material.
        void SetMaterialAttribute(const QStringList &params);

    private slots:
        /// New scene has been created
        void OnSceneAdded(const QString &name);

        /// Scene is about to be removed
        void OnSceneRemoved(const QString &name);

    private:
        RendererPtr renderer;  ///< Renderer
    };
}

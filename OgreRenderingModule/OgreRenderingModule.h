// For conditions of distribution and use, see copyright notice in license.txt

#pragma once

#include "IModule.h"
#include "OgreModuleApi.h"
#include "OgreModuleFwd.h"

namespace OgreRenderer
{
    class RendererSettingsWindow;

    /// A renderer module using Ogre
    /** \defgroup OgreRenderingModuleClient OgreRenderingModule Client Interface
        This page lists the public interface of the OgreRenderingModule.

        For details on how to use the public interface, see \ref OgreRenderingModule "Using the Ogre renderer module"

        \bug Ogre assert fail when viewing a mesh that contains a reference to non-existing skeleton.
    */
    class OGRE_MODULE_API OgreRenderingModule : public QObject, public IModule
    {
        Q_OBJECT

    public:
        OgreRenderingModule();
        virtual ~OgreRenderingModule();

        virtual void Load();
        virtual void PreInitialize();
        virtual void Initialize();
        virtual void PostInitialize();
        virtual void Uninitialize();
        virtual void Update(f64 frametime);

        /// Returns the renderer.
        RendererPtr GetRenderer() const { return renderer; }

    public slots:
        /// Shows renderer settings window.
        void ShowSettingsWindow();

        /// Prints renderer stats to console.
        void ConsoleStats();

        /// Sets attribute value for material.
        void SetMaterialAttribute(const StringVector &params);

    private:
        RendererPtr renderer;  ///< Renderer
        RendererSettingsWindow *settingsWindow; ///< Renderer settings window.
    };
}

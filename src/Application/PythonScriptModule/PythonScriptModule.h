// For conditions of distribution and use, see copyright notice in license.txt

#pragma once

#include "PythonFwd.h"
#include "PythonModuleApi.h"

#include "IModule.h"
#include "PythonQtScriptingConsole.h"

#include <QObject>
#include <QList>
#include <QString>
#include <QStringList>
#include <QVariantMap>

#include <Python.h>

namespace PythonScript
{
    /// A scripting module using Python
    class PY_MODULE_API PythonScriptModule : public IModule
    {
    
    Q_OBJECT
    
    public slots:
        /// Prepares Python script instance used with EC_Script for execution.
        /// The script is executed instantly only if the runOnLoad attribute of the script EC is true.
        /// @param scriptAsset Script asset.
        void LoadScript(const std::vector<ScriptAssetPtr> &newScripts);

        /// Slot for FrameAPI::Updated signal
        void UpdatePython(float frametime);

        /// Get renderer
        OgreRenderer::Renderer* GetRenderer() const;

        /// Get active camera Entity*
        Entity* GetActiveCamera() const;

        /// Get Scene* by name.
        /// \todo this function can be found directly from SceneAPI, remove this?
        Scene* GetScene(const QString &name) const;

        /// Reset a dynamic qt property value if exists.
        /// \todo Really needed, remove?
        void ResetQtDynamicProperty(QObject* qobj, char* propname);
        
        /// Get a python module InputContext*.
        InputContext* GetInputContext() const { return input.get(); }

        /// Create a new InputContext* with name and priority.
        InputContext* CreateInputContext(const QString &name, int priority = 100);

        /// Slot callbacks for console commands
        void ShowConsole();
        void ConsoleRunString(const QStringList &params);
        void ConsoleRunFile(const QStringList &params);
        void ConsoleReset(const QStringList &params);

    private slots:
        /// Scene added signal handler.
        void OnSceneAdded(const QString &name);

        /** Called when new component is added to the active scene.
            Currently used for handling EC_Script.
            @param entity Entity for which the component was added.
            @param component The added component.
         */
        void OnComponentAdded(Entity *entity, IComponent *component);

        /** Called when component is removed from the active scene.
            Currently used for handling EC_Script.
            @param entity Entity from which the component was removed.
            @param component The removed component.
        */
        void OnComponentRemoved(Entity *entity, IComponent *component);

    public:
        PythonScriptModule();
        virtual ~PythonScriptModule();

        // Module interface overrides
        virtual void Load();
        virtual void Unload();
        virtual void Initialize();
        virtual void PostInitialize();
        virtual void Uninitialize();
        
        /// Returns the currently initialized PythonScriptModule.
        static PythonScriptModule *GetInstance();

        /// Wrap QObject* to PyObject*.
        PyObject *WrapQObject(QObject* qobj) const;

    private:
        /// Static instance of ourselves.
        static PythonScriptModule *pythonScriptModuleInstance_;

        /// Python engine.
        PythonEnginePtr engine_;

        /// Tracking boolean if python qt is initialized.
        bool pythonQtInitialized_;

        /// The hook to the python-written module manager that passes events on
        PyObject *pmmModule, *pmmDict, *pmmClass, *pmmInstance;
        PyObject *pmmArgs, *pmmValue;
        
        /// The default input context for python code to access. This context operates below
        /// the Qt windowing priority.
        InputContextPtr input;

        /// List of created InputContextPtrs.
        QList<InputContextPtr> createdInputs_;
    };

    static PythonScriptModule *self() { return PythonScriptModule::GetInstance(); }
}

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
    
    public:
        /// Constuctor
        PythonScriptModule();

        /// Deconstructor. 
        /// Stops Python and finalizes it with 
        /// Py_Finalize() and does PythonQt cleanup.
        virtual ~PythonScriptModule();

        /// IModule override
        virtual void Load();

        /// IModule override
        virtual void PostInitialize();

        /// IModule override
        virtual void Uninitialize();

    public slots:
        /// Prepares Python script instance used with EC_Script for execution.
        /// The script is executed instantly only if the runOnLoad attribute of the script EC is true.
        /// @param scriptAsset Script asset.
        void LoadScript(const std::vector<ScriptAssetPtr> &newScripts);

        /// Get Server
        QObject *GetServer() const;

        /// Get Client
        QObject *GetClient() const;

        /// Get renderer
        OgreRenderer::Renderer* GetRenderer() const;

        /// Get active camera Entity.
        Entity* GetActiveCamera() const;

        /// Create a new InputContext* with name and priority.
        InputContext* CreateInputContext(const QString &name, int priority = 100);

        /// Slot callbacks for console commands
        void ShowConsole();
        void ConsoleRunString(const QStringList &params);
        void ConsoleRunFile(const QStringList &params);
        void ConsoleRestartPython(const QStringList &params);

        /// Slot for our tundra.py to call on its logging implementation.
        /// @param logType Type of login print = INFO|WARNING|DEBUG|ERROR|FATAL.
        /// @param logMessage Message to print out.
        void PythonPrintLog(const QString &logType, const QString &logMessage);

    private slots:
        /// Load startup plugins from runtime xml config.
        void LoadStartupScripts();

        /// Start PythonQt.
        void StartPythonQt();

        /* Circuits Module Manager start and stop. I have implemented Tundra startup config xml parsing
         * that can have pyplugins. I don't see that we need circuits anymore for anything. tundra.py exposing the framework
         * plus startup py plugin loading should do the trick!
         * 
         * \todo Remove below funtions when decided.

        /// Start Python ModuleManager.
        void StartPythonModuleManager();

        /// Stop Python ModuleManager.
        void StopPythonModuleManager();

        /// Slot for FrameAPI::Updated signal
        void UpdatePythonModuleManager(float frametime);
        */

        /// Add path to python for imports.
        void AddSystemPath(const QString &path);

        /// Run code string.
        void RunString(const QString &codeStr);

        /// Run a script file. Path can be absolute or relative.
        void RunScript(const QString &scriptname);

        /// Slot for getting python std out from PythonQt.
        void OnPythonQtStdOut(const QString &str);

        /// Slot for getting python std err from PythonQt.
        void OnPythonQtStdErr(const QString &str);

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

    private:
        /// Tracking boolean if python qt is initialized.
        bool pythonQtStarted_;

        /// The hook to the python-written module manager that passes events on
        //PyObject *pyTundraModule_;
        //PyObject *pyTundraDict_;
        //PyObject *pyTundraClass_;
        //PyObject *pyTundraInstance_;

        /// List of created InputContextPtrs.
        QList<InputContextPtr> createdInputs_;
    };
}

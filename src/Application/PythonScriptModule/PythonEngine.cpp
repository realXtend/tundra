// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "Application.h"
#include "PythonEngine.h"
#include "PythonScriptModule.h"
#include "LoggingFunctions.h"

#include <QDir>
#include <stdio.h>

#include "MemoryLeakCheck.h"

namespace PythonScript
{
    QString PythonEngine::PYTHON_PLUGINS_DIRECTORY = "pyplugins";

    PythonEngine::PythonEngine(Framework* framework) :
        framework_(framework)
    {
    }

    PythonEngine::~PythonEngine()
    {
        /// \todo de-init/free here, implement?
    }

    void PythonEngine::Initialize()
    {
        QDir pythonPlugins(Application::InstallationDirectory() + PYTHON_PLUGINS_DIRECTORY);
        QDir pythonLibrary(pythonPlugins.absoluteFilePath("python/"));

        if (!Py_IsInitialized())
        {
            Py_Initialize();

            // Add Tundra python plugins source location.
            AddSystemPath(pythonPlugins.absolutePath());
            AddSystemPath(pythonPlugins.absoluteFilePath("lib"));

            // Add Python Library DLL and on windows pass whole python as a archive file.
            /// \todo Is the 'DLLs' really needed also outside windows?
            AddSystemPath(pythonLibrary.absoluteFilePath("DLLs"));
#ifdef _WIN32            
            AddSystemPath(pythonLibrary.absoluteFilePath("Python26.zip"));
#endif
        }
        else
            LogInfo("PythonEngine::Initialize(): Python already initialized in PythonScriptModule init!");
    }

    void PythonEngine::Uninitialize()
    {
        /// Check ~PythonEngine()
    }

    void PythonEngine::AddSystemPath(const QString &path)
    {
        RunString("import sys; sys.path.append('" + path + "');");
    }

    void PythonEngine::RunString(const QString &codestr, const QVariantMap &context)
    {
        /// \note: Doesn't handle extra context given, that was added to the interface for js can be added here too if needed.
        PyRun_SimpleString(codestr.toAscii().data());
    }

    void PythonEngine::RunScript(const QString &scriptname)
    {
        FILE *fp = fopen(scriptname.toAscii().data(), "r");
        if (!fp)
        {
            LogInfo("PythonEngine::RunScript(): Failed to open script " + scriptname.toStdString());
            return;
        }

        PyRun_SimpleFile(fp, scriptname.toAscii().data());
        fclose(fp);
    }
}

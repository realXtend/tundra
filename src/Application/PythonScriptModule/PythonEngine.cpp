// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "PythonEngine.h"
#include "PythonScriptModule.h"
#include "LoggingFunctions.h"

#include <stdio.h>

#include "MemoryLeakCheck.h"

namespace PythonScript
{
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
        if (!Py_IsInitialized())
        {
            Py_Initialize();
#ifdef _WIN32
            //for some reason setting env vars has no effect when running from inside visual studio,
            //so for VS use, the PYTHONHOME env var had to be set in the project file
            //.. that is inconvenient, so changed the path manipulation to back here.
            RunString("import sys; sys.path.append('pymodules/python26_Lib.zip');");
#endif
            RunString("import sys; sys.path.append('pymodules');"); //XXX change to the c equivalent
            RunString("import sys; sys.path.append('pymodules/lib');"); // libraries directory
            RunString("import sys; sys.path.append('pymodules/DLLs');"); // dll directory (PYTHONHOME doesent seem to work to get these)
        }
        else
        {
            //LogWarning() //XXX add module ref here to be able to do logging
            LogInfo("PythonEngine::Initialize(): Python already initialized in PythonScriptModule init!");
        }
    }

    void PythonEngine::Uninitialize()
    {
        /// Check ~PythonEngine()
    }

    void PythonEngine::RunString(const QString &codestr, const QVariantMap &context)
    {
        PyRun_SimpleString(codestr.toAscii().data());
        //\note: doesn't handle extra context given, that was added to the interface for js can be added here too if needed.
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

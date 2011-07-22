// For conditions of distribution and use, see copyright notice in license.txt

#pragma once

#include "PythonFwd.h"

namespace PythonScript
{    
    /// Python code runner, created by PythonScriptModule.
    class PythonEngine // : public Foundation::ScriptServiceInterface
    {

    public:
        PythonEngine(Framework* framework);
        virtual ~PythonEngine();

        void Initialize();
        void Uninitialize();

        virtual void RunScript(const QString &scriptname);
        virtual void RunString(const QString &codestr, const QVariantMap &context = QVariantMap());

    private:
        Framework* framework_;        
    };
}

// For conditions of distribution and use, see copyright notice in license.txt

#pragma once

#include "PythonFwd.h"
#include <QString>
#include <QVariantMap>

namespace PythonScript
{    
    /// Python code runner, created by PythonScriptModule.
    class PythonEngine
    {

    public:
        PythonEngine(Framework* framework);
        virtual ~PythonEngine();

        void Initialize();
        void Uninitialize();

        void AddSystemPath(const QString &path);

        virtual void RunScript(const QString &scriptname);
        virtual void RunString(const QString &codestr, const QVariantMap &context = QVariantMap());

        static QString PYTHON_PLUGINS_DIRECTORY;
    private:
        Framework* framework_;        
    };
}

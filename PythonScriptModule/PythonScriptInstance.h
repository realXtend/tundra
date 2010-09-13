/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   IScriptInterface.h
 *  @brief  Python script instance used with EC_Script.
 */

#ifndef incl_PythonScriptModule_PythonScriptInstance_h
#define incl_PythonScriptModule_PythonScriptInstance_h

#include "IScriptInstance.h"

#include <QString>

#include <PythonQtObjectPtr.h>

/// Python script instance used with EC_Script.
class PythonScriptInstance : public IScriptInstance
{
public:
    /// Default constuctor.
    PythonScriptInstance(const QString &filename);

    /// Destructor.
    virtual ~PythonScriptInstance() {}

    /// IScriptInstance override.
    void Reload();

    /// IScriptInstance override.
    void Unload();

    /// IScriptInstance override.
    void Run();

    /// IScriptInstance override.
    void Stop();

private:
    PythonQtObjectPtr context_;

    QString filename_;
};

#endif

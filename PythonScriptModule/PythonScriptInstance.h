/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   PythonScriptInstance.h
 *  @brief  Python script instance used with EC_Script.
 */

#ifndef incl_PythonScriptModule_PythonScriptInstance_h
#define incl_PythonScriptModule_PythonScriptInstance_h

#include "IScriptInstance.h"

#include <QString>

#include <PythonQtObjectPtr.h>

namespace Scene
{
    class Entity;
}

/// Python script instance used with EC_Script.
class PythonScriptInstance : public IScriptInstance
{
public:
    /// Constructs new script instance. Creates new module/context for the script file.
    /** @param filename Filename of the script (include path and file extension).
        @param entity Parent entity.
    */
    PythonScriptInstance(const QString &filename, Scene::Entity *entity);

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
    /// Python context for this script instance.
    PythonQtObjectPtr context_;

    /// Script filename.
    QString filename_;

    /// Python module name for the script file.
    QString moduleName_;
};

#endif

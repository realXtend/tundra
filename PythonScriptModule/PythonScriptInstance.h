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
    void Load();

    /// IScriptInstance override.
    void Unload();

    /// IScriptInstance override.
    void Run();

    /// IScriptInstance override.
    virtual QString GetLoadedScriptName() const { return filename_; }

private:
    PythonQtObjectPtr context_; ///< Python context for this script instance.
    QString filename_; ///< Script filename.
    QString moduleName_; ///< Python module name for the script file.
};

#endif

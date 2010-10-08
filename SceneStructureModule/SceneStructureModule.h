/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   SceneStructureModule.h
 *  @brief  
 */

#ifndef incl_SceneStructureModule_SceneStructureModule_h
#define incl_SceneStructureModule_SceneStructureModule_h

#include "IModule.h"
#include "ModuleLoggingFunctions.h"

class SceneStructureWindow;

///
/**
*/
class SceneStructureModule : public QObject, public IModule
{
    Q_OBJECT

public:
    /// Default constructor.
    SceneStructureModule();

    /// Destructor.
    ~SceneStructureModule();

    /// IModule override.
    void PostInitialize();

    MODULE_LOGGING_FUNCTIONS

    /// Returns name of this module. Needed for logging.
    static const std::string &NameStatic() { return typeNameStatic; }

private:
    //! Type name of the module.
    static std::string typeNameStatic;

    /// Scene structure window.
    SceneStructureWindow *sceneWindow;

    /// Input context.
    boost::shared_ptr<InputContext> inputContext;

private slots:
    /// Shows scene structure window.
    void ShowSceneStructureWindow();

    /// Handles KeyPressed() signal from input context.
    /** @param e Key event.
    */
    void HandleKeyPressed(KeyEvent *e);
};

#endif

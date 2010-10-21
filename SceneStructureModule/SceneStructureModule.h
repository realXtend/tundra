/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   SceneStructureModule.h
 *  @brief  Provides Scene Structure window and raycast drag-and-drop import of
 *          mesh, .scene, .xml and .nbf files to the main window.
 */

#ifndef incl_SceneStructureModule_SceneStructureModule_h
#define incl_SceneStructureModule_SceneStructureModule_h

#include "IModule.h"
#include "Vector3D.h"

class QDragEnterEvent;
class QDragMoveEvent;
class QDropEvent;

class SceneStructureWindow;

/// Provides Scene Structure window and raycast drag-and-drop import
/// of .mesh, .scene, .xml and .nbf files to the main window.
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

public slots:
    /// Instantiates new content from file to the scene.
    /** @param filename File name.
        @param worldPos Destination in-world position. If zero vector is given, the position is asked with a simple dialog.
        @param clearScene Do we want to clear the scene before adding new content.
        @return List of created entities.
    */
    QList<Scene::Entity *> InstantiateContent(const QString &filename, Vector3df worldPos, bool clearScene);

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

    /// Handles main window drag enter event.
    /** If event's MIME data contains URL's we accept it.
        @param e Event.
    */
    void HandleDragEnterEvent(QDragEnterEvent *e);

    /// Handles main window drag move event.
    /** If event's MIME data contains URL's we accept it.
        @param e Event.
    */
    void HandleDragMoveEvent(QDragMoveEvent *e);

    /// Handles drop event.
    /** If event's MIME data contains URL's we accept it.
        @param e Event.
    */
    void HandleDropEvent(QDropEvent *e);
};

#endif

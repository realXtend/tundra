/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   SceneStructureModule.h
 *  @brief  Provides Scene Structure window and raycast drag-and-drop import of
 *          .mesh, .scene, .txml and .tbinfiles to the main window.
 */

#ifndef incl_SceneStructureModule_SceneStructureModule_h
#define incl_SceneStructureModule_SceneStructureModule_h

#include "IModule.h"
#include "Vector3D.h"

class QDragEnterEvent;
class QDragMoveEvent;
class QDropEvent;

class SceneStructureWindow;

struct SceneDesc;

/// Provides Scene Structure window and raycast drag-and-drop import
/// of .mesh, .scene, .txml and .tbin files to the main window.
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
        @param worldPos Destination in-world position.
        @param clearScene Do we want to clear the scene before adding new content.
        @param queryPosition Do we want to query the position from user.
        @return List of created entities.
    */
    QList<Scene::Entity *> InstantiateContent(const QString &filename, Vector3df worldPos, bool clearScene,
        bool queryPosition = false);

    /// This is an overloaded function
    /** Uses scene description structure to filter unwanted content.
        @param filename File name.
        @param worldPos Destination in-world position.
        @param clearScene Do we want to clear the scene before adding new content.
        @param desc Scene description filter
        @param queryPosition Do we want to query the position from user.
        @return List of created entities.
    */
    QList<Scene::Entity *> InstantiateContent(const QString &filename, Vector3df worldPos, const SceneDesc &desc,
        bool clearScene, bool queryPosition = false);

    /// Centralizes group of entities around same center point. The entities must have EC_Placeable component present.
    /** @param pos Center point for entities.
        @param entities List of entities.
    */
    static void CentralizeEntitiesTo(const Vector3df &pos, const QList<Scene::Entity *> &entities);

    /// Returns true of the file extension of @c filename is supported file type for importing.
    /** @param filename File name.
    */
    static bool IsSupportedFileType(const QString &filename);

private:
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
    /** If event's MIME data contains URL which path has supported file extension we accept it.
        @param e Event.
    */
    void HandleDragEnterEvent(QDragEnterEvent *e);

    /// Handles main window drag move event.
    /** If event's MIME data contains URL which path has supported file extension we accept it.
        @param e Event.
    */
    void HandleDragMoveEvent(QDragMoveEvent *e);

    /// Handles drop event.
    /** If event's MIME data contains URL which path has supported file extension we accept it.
        @param e Event.
    */
    void HandleDropEvent(QDropEvent *e);
};

#endif

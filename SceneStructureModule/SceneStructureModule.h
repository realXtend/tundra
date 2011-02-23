/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   SceneStructureModule.h
 *  @brief  Provides Scene Structure and Assets windows and raycast drag-and-drop import of
 *          various content file formats to the main window.
 */

#ifndef incl_SceneStructureModule_SceneStructureModule_h
#define incl_SceneStructureModule_SceneStructureModule_h

#include "IModule.h"
#include "Vector3D.h"
#include "AssetAPI.h"
#include "AssetReference.h"

#include <QPointer>

class QDragEnterEvent;
class QDragMoveEvent;
class QDropEvent;

class SceneStructureWindow;
class AssetsWindow;
struct SceneDesc;

class EC_Mesh;

struct SceneMaterialDropData
{
    EC_Mesh *mesh;
    AssetReferenceList materials;
    QList<uint> affectedIndexes;
};

/// Provides Scene Structure and Assets windows and raycast drag-and-drop import of
/// various content file formats to the main window.
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
        @return List of created entities.
    */
    QList<Scene::Entity *> InstantiateContent(const QString &filename, Vector3df worldPos, bool clearScene);

    /// This is an overloaded function
    /** Uses scene description structure to filter unwanted content.
        @param filename File name.
        @param worldPos Destination in-world position.
        @param clearScene Do we want to clear the scene before adding new content.
        @param desc Scene description filter
        @return List of created entities.
    */
    QList<Scene::Entity *> InstantiateContent(const QString &filename, Vector3df worldPos, const SceneDesc &desc, bool clearScene);

    QList<Scene::Entity *> InstantiateContent(const QStringList &filenames, Vector3df worldPos, const SceneDesc &desc, bool clearScene);

    /// Centralizes group of entities around same center point. The entities must have EC_Placeable component present.
    /** @param pos Center point for entities.
        @param entities List of entities.
    */
    static void CentralizeEntitiesTo(const Vector3df &pos, const QList<Scene::Entity *> &entities);

    /// Returns true of the file extension of @c fileRef is supported file type for importing.
    /** @param fileRef File name or url.
    */
    static bool IsSupportedFileType(const QString &fileRef);

    /// Returns true of the file extension of @c fileRef is supported material file for importing.
    /** @param fileRef File name or url.
    */
    static bool IsMaterialFile(const QString &fileRef);

    /// Returns true if the @c fileRef is a http:// or https:// scema url. 
    /** @param fileRef File name or url.
    */
    static bool IsUrl(const QString &fileRef);

    /// Cleans the @c fileRef
    /** @param fileRef File name or url.
    */
    static void CleanReference(QString &fileRef);

    /// Shows Scene Structure window.
    void ShowSceneStructureWindow();

    /// Shows Assets window.
    void ShowAssetsWindow();

private:
    SceneStructureWindow *sceneWindow; ///< Scene Structure window.
    QPointer<AssetsWindow> assetsWindow;///< Assets window.
    boost::shared_ptr<InputContext> inputContext; ///< Input context.

    SceneMaterialDropData materialDropData;

private slots:
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

    /// Handles material drop event.
    /** If event's MIME data contains a single URL which is a material the drop is redirected to this function.
        @param e Event.
        @param materialRef Dropped material file or url.
    */
    void HandleMaterialDropEvent(QDropEvent *e, const QString &materialRef);

    /// Finishes a material drop
    /** @param contentAdd If drop content should be appliead or not
    */
    void FinishMaterialDrop(bool apply, const QString &materialBaseUrl);

    void HandleSceneDescLoaded(AssetPtr asset);

    void HandleSceneDescFailed(IAssetTransfer *transfer, QString reason);
};

#endif

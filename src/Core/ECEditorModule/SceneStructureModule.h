/**
    For conditions of distribution and use, see copyright notice in LICENSE

    @file   SceneStructureModule.h
    @brief  Provides UIs for scene and asset maintenance and content import. */

#pragma once

#include "IModule.h"
#include "SceneFwd.h"
#include "InputFwd.h"
#include "AssetFwd.h"
#include "AssetReference.h"
#include "Math/MathFwd.h"

#include <QPointer>
#include <QWidget>
#include <QLabel>
#include <QHash>

class QDragEnterEvent;
class QDragMoveEvent;
class QDropEvent;

class SceneStructureWindow;
class AssetsWindow;
class KeyBindingsConfigWindow;

class EC_Mesh;
class ECEditorWindow;

/// Provides UIs for scene and asset maintenance and content import.
/** Also implements raycast drag-and-drop import of various content file formats to the main window. */
class SceneStructureModule : public IModule
{
    Q_OBJECT

public:
    SceneStructureModule();
    ~SceneStructureModule();
    void Initialize();
    void Uninitialize();

public slots:
    /// Starts instantiation of new content from files to the scene by using the AddContentWindow.
    /** @param filenames List of content files.
        @param worldPos Destination in-world position.
        @param clearScene Do we want to clear the scene before adding new content.
        @return List of created entities.
        @todo clearScene not used currently for anything. */
    void InstantiateContent(const QStringList &filenames, const float3 &worldPos, bool clearScene);

    /// @overload
    /** @param filenames Content filename.
        @todo clearScene not used currently for anything. */
    void InstantiateContent(const QString &filename, const float3 &worldPos, bool clearScene);

    /// Centralizes group of entities around same center point. The entities must have EC_Placeable component present.
    /** @param pos Center point for entities.
        @param entities List of entities. */
    static void CentralizeEntitiesTo(const float3 &pos, const QList<Entity *> &entities);

    /// Returns true of the file extension of @c fileRef is supported file type for importing.
    /** @param fileRef File name or url. */
    static bool IsSupportedFileType(const QString &fileRef);

    /// Returns true of the file extension of @c fileRef is supported material file for importing.
    /** @param fileRef File name or url. */
    static bool IsMaterialFile(const QString &fileRef);

    /// Returns true if the @c fileRef is a http:// or https:// scema url.
    /** @param fileRef File name or url. */
    static bool IsUrl(const QString &fileRef);

    /// Cleans the @c fileRef
    /** @param fileRef File name or url. */
    static void CleanReference(QString &fileRef);

    /// Toggles visibility of Scene Structure window.
    void ToggleSceneStructureWindow();

    /// Toggles visibility of Assets window.
    void ToggleAssetsWindow();

    /// Toggles visibility of Key Bindings window.
    void ToggleKeyBindingsWindow();

private:
    void SaveWindowPosition(QWidget *widget, const QString &settingName);
    void LoadWindowPosition(QWidget *widget, const QString &settingName);

    struct SceneMaterialDropData
    {
        SceneMaterialDropData() : mesh(0) {}
        EC_Mesh *mesh;
        AssetReferenceList materials;
        QList<uint> affectedIndexes;
    };

    QPointer<SceneStructureWindow> sceneWindow;
    QPointer<AssetsWindow> assetsWindow;
    QPointer<KeyBindingsConfigWindow> keyBindingsWindow;
    QPointer<ECEditorWindow> syncedECEditor;
    shared_ptr<InputContext> inputContext;

    SceneMaterialDropData materialDropData;
    QHash<QString, float3> urlToDropPos;

    QWidget *toolTipWidget;
    QLabel *toolTip;
    QString currentToolTipSource;
    QString currentToolTipDestination;

private slots:
    /// Handles KeyPressed() signal from input context.
    void HandleKeyPressed(KeyEvent *e);

    /// Handles main window drag enter event.
    /** If event's MIME data contains URL which path has supported file extension we accept it. */
    void HandleDragEnterEvent(QDragEnterEvent *e, QGraphicsItem *widget);

    /// Handles main window drag leave event.
    void HandleDragLeaveEvent(QDragLeaveEvent *e);

    /// Handles main window drag move event.
    /** If event's MIME data contains URL which path has supported file extension we accept it. */
    void HandleDragMoveEvent(QDragMoveEvent *e, QGraphicsItem *widget);

    /// Handles drop event.
    /** If event's MIME data contains URL which path has supported file extension we accept it. */
    void HandleDropEvent(QDropEvent *e, QGraphicsItem *widget);

    /// Handles material drop event.
    /** If event's MIME data contains a single URL which is a material the drop is redirected to this function.
        @param e Event.
        @param materialRef Dropped material file or url. */
    void HandleMaterialDropEvent(QDropEvent *e, const QString &materialRef);

    /// Finishes a material drop
    /** @param apply If drop content should be appliead or not. */
    void FinishMaterialDrop(bool apply, const QString &materialBaseUrl);

    void HandleSceneDescLoaded(AssetPtr asset);

    void HandleSceneDescFailed(IAssetTransfer *transfer, QString reason);

    /// Decorates entities in SceneStructureWindow to reflect the selection of currently active ECEditorWindow.
    void SyncSelectionWithEcEditor(ECEditorWindow *);
};

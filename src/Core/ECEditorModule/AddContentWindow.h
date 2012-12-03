/**
    For conditions of distribution and use, see copyright notice in LICENSE

    @file   AddContentWindow.h
    @brief  Window for adding new content and uploading assets. */

#pragma once

#include <QWidget>
#include <QTreeWidget>

#include "ECEditorModuleApi.h"
#include "SceneFwd.h"
#include "AssetFwd.h"
#include "SceneDesc.h"
#include "Math/float3.h"

class QPushButton;
class QComboBox;
class QTreeWidgetItem;
class QProgressBar;
class QLabel;

typedef QList<QTreeWidgetItem *> SelectedItemsList;

/// Tree widget that allows multiselection and handles 'space' key press for checking/unchecking checkboxes
/// @cond PRIVATE
class EntityAndAssetTreeWidget : public QTreeWidget
{
    Q_OBJECT
public:
    EntityAndAssetTreeWidget(QWidget *parent = 0);
protected:
    virtual void keyPressEvent(QKeyEvent* event);
private:
    void ToggleCheckedState(bool checkAllInsteadOfToggle);
};
/// @endcond

/// Window for adding new content and uploading assets.
/** The window is modal and is deleted when it's closed. */
class ECEDITOR_MODULE_API AddContentWindow : public QWidget
{
    Q_OBJECT

public:
    /// Constructs the window.
    /** @param dest Destination scene.
        @param parent Parent widget. */
    AddContentWindow(const ScenePtr &dest, QWidget *parent = 0);
    ~AddContentWindow();

    /// Adds scene description to be shown in the window.
    /** @param desc Scene description. */
    void AddDescription(const SceneDesc &desc);

    /// This is an overloded function.
    /** Adds multiple scene descriptions to be shown in the window. 
        @param descs Scene descriptions. */
    void AddDescription(const QList<SceneDesc> &descs);

    /// Adds assets to be shown in the window.
    /** @param fileNames List of files. */
    void AddAssets(const QStringList &fileNames);

    /// Sets a position which will be applied to the created entities.
    /** @param pos Position. */
    void SetContentPosition(const float3 &pos) { position = pos; }

signals:
    /// Emitted when possible entity creations are completed.
    /** @param entities */
    void EntitiesCreated(const QList<Entity *> &entities);

    /// Emitted when possible asset uploads are completed.
    /** @param storage Storage where assets were uploaded.
        @param numSuccessful Number of successful uploads.
        @param numFailed Number of failed uploads.*/
    void AssetUploadCompleted(const AssetStoragePtr &storage, int numSuccessful, int numFailed);

    /// @deprecated use AssetUploadCompleted and EntitiesCreated instead @todo Remove
    void Completed(bool contentAdded, const QString &uploadBaseUrl);

protected:
    /// QWidget override.
    void showEvent(QShowEvent *e);

private:
    Q_DISABLE_COPY(AddContentWindow)

    /// Creates entity items to the entity tree widget.
    /** @param entityDescs List of entity descriptions. */
    void AddEntities(const QList<EntityDesc> &entityDescs);

    /// Creates asset items to the asset tree widget.
    /** @param sceneDescs Source scene desc.
        @param assetDescs List of assets descriptions. */
    void AddAssets(const SceneDesc &sceneDesc, const SceneDesc::AssetMap &assetDescs);

    /// Rewrites values of AssetReference or AssetReferenceList attributes.
    /** @param sceneDesc Scene description.
        @param dest Destination asset storage.
        @param useDefaultStorage Do we want to use the default asset storage. */
    void RewriteAssetReferences(SceneDesc &sceneDesc, const AssetStoragePtr &dest, bool useDefaultStorage);

    /// Returns name of the currently selected asset storage.
    QString CurrentStorageName() const;

    AssetStoragePtr CurrentStorage() const;

    /// Generates contents of asset storage combo box. Sets default storage selected as default.
    void GenerateStorageComboBoxContents();

    QTreeWidget *entityTreeWidget;
    QTreeWidget *assetTreeWidget;
    Framework *framework;
    SceneWeakPtr scene; ///< Destination scene.
    QList<SceneDesc> sceneDescs; ///< Current scene description(s) shown on the window.
    QPushButton *addContentButton;
    QPushButton *cancelButton;
    QComboBox *storageComboBox;
    float3 position; ///< Centralization position for instantiated context (if used).

    // Uploading
    QLabel *uploadStatusLabel;
    QProgressBar *uploadProgressBar;
    int uploadProgressStep;
    int numFailedUploads;
    int numSuccessfulUploads;
    int numTotalUploads; ///< Number of uploads initiated, set at UploadAssets()

    // Entities add
    QLabel *entityStatusLabel;
    QProgressBar *entityProgressBar;

    // Parent widget
    QWidget *entityView;
    QWidget *assetView;

    // Selected entities and assets
    SceneDesc filteredDesc;

private slots:
    /// Checks all entity check boxes.
    void SelectAllEntities();

    /// Unchecks all entity check boxes.
    void DeselectAllEntities();

    /// Checks all asset check boxes.
    void SelectAllAssets();

    /// Unchecks all asset check boxes.
    void DeselectAllAssets();

    /// Start content creation and asset uploading.
    void AddContent();

    /// Returns description of the currently selected/filtered content.
    SceneDesc CurrentContent() const;

    /// Creates new scene description with ui check box selections.
    void CreateNewDesctiption();

    bool CheckForStorageValidity();

    /// Starts uploading of assets, if applicable.
    void UploadAssets();

    /// Add entities to scene.
    bool CreateEntities();

    /// @todo Remove from here and create a utility function UiMainWindow::CenterWidget
    void CenterToMainWindow();

    /// Set entity related widgets visibility.
    void SetEntitiesVisible(bool visible);

    /// Set assets related widgets visibility.
    void SetAssetsVisible(bool visible);

    /// Checks if tree widget column is editable.
    /** @param item Item which was double-clicked.
        @param column Column index. */
    void CheckIfColumnIsEditable(QTreeWidgetItem *item, int column);

    /// Rewrites the destination names of all assets in the UI accordingly to the selected asset storage.
    void RewriteDestinationNames();

    /// Calls HandleUploadProgress(true, transfer).
    void HandleUploadCompleted(IAssetUploadTransfer *transfer);
    /// Calls HandleUploadProgress(false, transfer)
    void HandleUploadFailed(IAssetUploadTransfer *transfer);
    /// Handles upload progress and updates the UI.
    void HandleUploadProgress(bool successful, IAssetUploadTransfer *transfer);
};

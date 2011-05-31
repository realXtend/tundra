/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   AddContentWindow.h
 *  @brief  Window for adding new content and assets.
 */

#pragma once

#include <QWidget>

#include "SceneFwd.h"
#include "AssetFwd.h"
#include "SceneDesc.h"
#include "Vector3D.h"

class QTreeWidget;
class QPushButton;
class QComboBox;
class QTreeWidgetItem;
class QProgressBar;
class QLabel;

/// Window for adding new content and assets.
/** The window is modal and is deleted when it's closed. */
class AddContentWindow : public QWidget
{
    Q_OBJECT

public:
    /// Constructs the window.
    /** @param fw Framework.
        @param dest Destination scene.
        @param parent Parent widget. */
    AddContentWindow(Framework *fw, const ScenePtr &dest, QWidget *parent = 0);

    /// Destructor.
    ~AddContentWindow();

    /// Adds scene description to be shown in the window.
    /** @param desc Scene description. */
    void AddDescription(const SceneDesc &desc);

    /// Adds multiple scene descriptions to be shown in the window.
    /** @param desc Scene description. */
//    void AddDescriptions(const QList<SceneDesc> &descs);

    /// Adds files to be shown in the window.
    /** @param fileNames List of files. */
    void AddFiles(const QStringList &fileNames);

    /// Add offset position which will be applied to the created entities.
    /** @param pos Position. */
    void AddPosition(const Vector3df &pos) { position = pos; }

signals:
    void Completed(bool contentAdded, const QString &uploadBaseUrl);

protected:
    void showEvent(QShowEvent *e);

private:
    Q_DISABLE_COPY(AddContentWindow)

    /// Creates entity items to the entity tree widget.
    /** @param entityDescs List of entity descriptions. */
    void AddEntities(const QList<EntityDesc> &entityDescs);

    /// Creates asset items to the asset tree widget.
    /** @param assetDescs List of assets descriptions. */
    void AddAssets(const SceneDesc::AssetMap &assetDescs);

    /// Rewrites values of AssetReference or AssetReferenceList attributes.
    /** @param sceneDesc Scene description.
        @param dest Destination asset storage.
        @param useDefaultStorage Do we want to use the default asset storage.
    */
    void RewriteAssetReferences(SceneDesc &sceneDesc, const AssetStoragePtr &dest, bool useDefaultStorage);

    /// Returns name of the currently selected asset storage.
    QString GetCurrentStorageName() const;

    /// Generates contents of asset storage combo box. Sets default storage selected as default.
    void GenerateStorageComboBoxContents();

    QTreeWidget *entityTreeWidget; ///< Tree widget showing entities.
    QTreeWidget *assetTreeWidget; ///< Tree widget showing asset references.
    Framework *framework; ///< Framework.
    SceneWeakPtr scene; ///< Destination scene.
    SceneDesc sceneDesc; ///< Current scene description shown on the window.
    QPushButton *addContentButton; ///< Add content button.
    QPushButton *cancelButton; ///< Cancel/close button.
    QComboBox *storageComboBox; ///< Asset storage combo box.
    Vector3df position; ///< Centralization position for instantiated context (if used).

    // Uploading
    QLabel *uploadStatus_;
    QProgressBar *uploadProgress_;
    int progressStep_;
    int failedUploads_;
    int successfullUploads_;
    int totalUploads_;

    // Entities add
    QLabel *entitiesStatus_;
    QProgressBar *entitiesProgress_;

    // Parent widget
    QWidget *parentEntities_;
    QWidget *parentAssets_;

    // Selected entities and assets
    SceneDesc newDesc_;

    // Bool for completion
    bool contentAdded_;

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

    /// Create new scene description with ui check box selections
    bool CreateNewDesctiption();

    /// Start uploading
    bool UploadAssets();

    /// Add entities to scene (automatically called when all transfers finish)
    void AddEntities();

    /// Centers this window to the app main window
    void CenterToMainWindow();

    /// Set entity related widgets visibility.
    void SetEntitiesVisible(bool visible);

    /// Set assets related widgets visibility.
    void SetAssetsVisible(bool visible);

    /// Closes the window.
    void Close();

    /// Checks if tree widget column is editable.
    /** @param item Item which was double-clicked.
        @param column Column index. */
    void CheckIfColumnIsEditable(QTreeWidgetItem *item, int column);

    /// Rewrites the destination names of all assets in the UI accordingly to the selected asset storage.
    void RewriteDestinationNames();

    /// Handles completed upload asset transfer.
    /** @param transfer Completed transfer. */
    void HandleUploadCompleted(IAssetUploadTransfer *transfer);

    /// Handles failed upload asset transfer.
    /** @param transfer Failed transfer. */
    void HandleUploadFailed(IAssetUploadTransfer *trasnfer);

    void UpdateUploadStatus(bool succesfull, const QString &assetRef);

    void CheckUploadTotals();
};

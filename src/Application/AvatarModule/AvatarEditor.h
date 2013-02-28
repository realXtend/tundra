// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "AvatarModule.h"
#include "AvatarModuleApi.h"
#include "FileUtils.h"
#include "SceneFwd.h"
#include "AssetFwd.h"

#include "ui_avatareditor.h"

#include <QWidget>

class QTabWidget;
class EC_Avatar;
class AvatarDescAsset;
class Framework;
typedef shared_ptr<AvatarDescAsset> AvatarDescAssetPtr;

/// Avatar editing window.
class AV_MODULE_API AvatarEditor : public QWidget, public Ui::AvatarEditor
{
    Q_OBJECT

public:
    /// Constructs and initializes the window.
    /** @param fw Framework.
        @parent parent Parent widget. */
    explicit AvatarEditor(Framework *fw, QWidget *parent = 0);
    ~AvatarEditor();

public slots:
    /// Rebuild edit view
    void RebuildEditView();

    /// Save avatar
    void SaveAvatar();

    /// Revert avatar edits
    void RevertAvatar();

    /// Change avatar's material
    void ChangeMaterial();

    /// Remove attachment
    void RemoveAttachment();

    /// Bone modifier scrollbar value handler
    void BoneModifierValueChanged(int value);

    /// Morph modifier scrollbar value handler
    void MorphModifierValueChanged(int value);

    /// Master appearance modifier scrollbar value handler
    void MasterModifierValueChanged(int value);

    /// Set avatar entity and asset to edit
    void SetEntityToEdit(EntityPtr entity);

    /// Set avatar asset to edit
    void SetAsset(AvatarDescAssetPtr desc);

    /// Open an AssetsWindow to choose an avatar asset to the editor. Saves previously opened avatar to previousAvatar_ for canceling the pick.
    void OpenAvatarAsset();

    /// Open an AssetsWindow to choose an attachment asset. Picking an asset adds it to the avatar. Changing selection and canceling don't cause any additional operations within AvatarEditor.
    void OpenAttachmentAsset();

protected:
    /// QWidget override.
    void changeEvent(QEvent* e);

private:
    /// Get the avatar entity, avatar component, and avatar description. If all are non-null, return true
    bool GetAvatarDesc(Entity*& entity, EC_Avatar*& avatar, AvatarDescAsset*& desc);

    /// Clear a panel
    void ClearPanel(QWidget* panel);

    /// Create or get a tabbed scrollarea panel
    QWidget* GetOrCreateTabScrollArea(QTabWidget* tabs, const std::string& name);

    Framework *framework; ///< Framework.
    /// Avatar entity to edit
    EntityWeakPtr avatarEntity_;
    /// Avatar asset to edit
    weak_ptr<AvatarDescAsset> avatarAsset_;
    /// Previous avatar asset, saved when the AssetsWindow for loading new avatar is opened
    weak_ptr<AvatarDescAsset> previousAvatar_;

    bool reverting_;

private slots:
    /// Load the avatar picked in OpenAvatarAsset to the editor. If the asset is not loaded, load it.
    /** @param asset The chosen avatar asset */
    void HandleAssetPicked(AssetPtr asset);

    /// When avatar pick is canceled, restore previously loaded avatar (from previousAvatar_)
    void RestoreOriginalValue();

    /// On successful transfer of avatar asset, load the asset to the editor
    /** @param asset Succesfully loaded avatar asset to be loaded into the editor. */
    void OnAssetTransferSucceeded(AssetPtr asset);

    /// On avatar asset transfer error, give an error message
    /** @param transfer IAssetTransfer
        @param reason Failure reason. */
    void OnAssetTransferFailed(IAssetTransfer *transfer, QString reason);

    /// Add the attachment picked in OpenAttachmentAsset to the avatar. If the asset is not loaded, load it.
    /** @param attachmentAsset The chosen attachment asset */
    void HandleAttachmentPicked(AssetPtr attachmentAsset);

    /// Add the attachment to the avatar after checking its validity.
    /** @param asset Succesfully loaded attachment asset to be added to the avatar.*/
    void AddAttachment(AssetPtr assetPtr);
};

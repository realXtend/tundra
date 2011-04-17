/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   OgreAssetEditorModule.h
 *  @brief  OgreAssetEditorModule.provides editing and previewing tools for
 *          OGRE assets such as meshes and material scripts.
 */

#ifndef incl_OgreAssetEditorModule_OgreAssetEditorModule_h
#define incl_OgreAssetEditorModule_OgreAssetEditorModule_h

#include "IModule.h"
#include "AssetFwd.h"
#include "OgreAssetEditorModuleApi.h"

#include <QObject>
#include <QAction>

class QMenu;

namespace Inventory
{
    class InventoryUploadEventData;
    class InventoryUploadBufferEventData;
}

class MaterialWizard;
class EditorManager;

class EditorAction : public QAction
{
    Q_OBJECT

public:
    ///
    /** @param asset
        @param text
        @param menu
    */
    EditorAction(const AssetPtr &asset, const QString &text, QMenu *menu);
    AssetWeakPtr asset;
};

class ASSET_EDITOR_MODULE_API OgreAssetEditorModule : public QObject, public IModule
{
    Q_OBJECT

public:
    OgreAssetEditorModule();
    ~OgreAssetEditorModule();

    // IModule overrides.
    void Initialize();
    void PostInitialize();
    void Uninitialize();
    void Update(f64 frametime);

    /// Returns name of this module. Needed for logging.
    static const std::string &NameStatic() { return typeNameStatic; }

public slots:
    /// Uploads new asset from file.
    /// @param data Inventory upload event data.
    void UploadFile(Inventory::InventoryUploadEventData *data);

    /// Uploads new asset from buffer.
    /// @param data Inventory buffer upload event data.
    void UploadBuffer(Inventory::InventoryUploadBufferEventData *data);

    bool IsSupportedAssetTypes(const QString &type) const;

//    void OpenAssetInEditor(const AssetPtr &asset);

private:
    Q_DISABLE_COPY(OgreAssetEditorModule);

    static std::string typeNameStatic; ///< Type name of this module.
    EditorManager *editorManager; ///< Editor manager.
    MaterialWizard *materialWizard; ///< Material wizard.

private slots:
    void OnContextMenuAboutToOpen(QMenu *menu, QList<QObject *> targets);

    void OpenAssetInEditor();
};

#endif

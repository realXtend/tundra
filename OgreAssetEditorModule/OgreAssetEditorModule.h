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

#include "OgreAssetEditorModuleApi.h"

#include <QObject>

class QMenu;

namespace Inventory
{
    class InventoryUploadEventData;
    class InventoryUploadBufferEventData;
}

class MaterialWizard;
class EditorManager;

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
    bool HandleEvent(event_category_id_t category_id, event_id_t event_id, IEventData* data);

    MODULE_LOGGING_FUNCTIONS

    /// Returns name of this module. Needed for logging.
    static const std::string &NameStatic() { return typeNameStatic; }

public slots:
    /// Uploads new asset from file.
    /// @param data Inventory upload event data.
    void UploadFile(Inventory::InventoryUploadEventData *data);

    /// Uploads new asset from buffer.
    /// @param data Inventory buffer upload event data.
    void UploadBuffer(Inventory::InventoryUploadBufferEventData *data);

private:
    Q_DISABLE_COPY(OgreAssetEditorModule);

    static std::string typeNameStatic; ///< Type name of this module.
    event_category_id_t frameworkEventCategory; ///< Framework event category.
    event_category_id_t inventoryEventCategory; ///< Inventory event category.
    event_category_id_t networkStateEventCategory; ///< NetworkState event category.
    EditorManager *editorManager; ///< Editor manager.
    MaterialWizard *materialWizard; ///< Material wizard.

private slots:
    void OnContextMenuAboutToOpen(QMenu *menu, QList<QObject *> targets);
};

#endif

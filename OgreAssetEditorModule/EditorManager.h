// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file   EditorManager.h
 *  @brief  EditorManager manages the editor windows.
 */

#ifndef incl_OgreAssetEditorModule_EditorManager_h
#define incl_OgreAssetEditorModule_EditorManager_h

#include "RexTypes.h"

#include <QObject>
#include <QMap>
#include <QPair>
#include <QString>

namespace OgreAssetEditor
{
    typedef QPair<QString, RexTypes::asset_type_t> EditorMapKey;
    typedef QMap<EditorMapKey, QObject *> EditorMap;
    typedef EditorMap::iterator EditorMapIter;
    typedef QMutableMapIterator<EditorMapKey, QObject *> MutableEditorMapIter;

    class EditorManager : public QObject
    {
        Q_OBJECT

    public:
        /// Default constuctor.
        EditorManager();

        /// Destructor.
        ~EditorManager();

    public slots:
        /// Adds new editor.
        /// @param inventory_id Inventory id.
        /// @param asset_type Asset type.
        /// @param editor Editor object.
        /// @return True if the editor was added succesfully, false otherwise.
        bool Add(const QString &inventory_id, RexTypes::asset_type_t asset_type, QObject *editor);

        /// Returns editor object.
        /// @param inventory_id Inventory id.
        /// @param asset_type Asset type.
        /// @return Editor object or null if not found.
        QObject *GetEditor(const QString &inventory_id, RexTypes::asset_type_t asset_type);

        ///
        /// @param inventory_id Inventory id.
        /// @param asset_type Asset type.
        /// @return True if the editor with matching id and asset type exists.
        bool Exists(const QString &inventory_id, RexTypes::asset_type_t asset_type);

        /// Deletes editor.
        /// @param inventory_id Inventory id.
        /// @return True if editor was found and deleted succesfully, false otherwise.
        bool Delete(const QString &inventory_id, RexTypes::asset_type_t asset_type);

        /// Deletes all editors.
        void DeleteAll();

    private:
        /// Returns editor object and removes it from the editor map.
        /// @param inventory_id Inventory id.
        /// @param asset_type Asset type.
        /// @return Editor object or null if not found.
        QObject *TakeEditor(const QString &inventory_id, RexTypes::asset_type_t asset_type);

        /// Map of currently existing editors.
        EditorMap editors_;
    };
}

#endif

// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file   EditorManager.h
 *  @brief  EditorManager manages the editor windows.
 */

#ifndef incl_OgreAssetEditorModule_EditorManager_h
#define incl_OgreAssetEditorModule_EditorManager_h

#include "RexTypes.h"

#include <QWidget>
#include <QMap>
#include <QPair>
#include <QString>

namespace Naali
{
    typedef QPair<QString, asset_type_t> EditorMapKey;
    typedef QMap<EditorMapKey, QWidget *> EditorMap;
    typedef EditorMap::iterator EditorMapIter;
    typedef QMutableMapIterator<EditorMapKey, QWidget *> MutableEditorMapIter;

    class EditorManager : public QWidget
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
        bool Add(const QString &inventory_id, asset_type_t asset_type, QWidget *editor);

        /// Returns editor object.
        /// @param inventory_id Inventory id.
        /// @param asset_type Asset type.
        /// @return Editor object or null if not found.
        QWidget *GetEditor(const QString &inventory_id, asset_type_t asset_type);

        /// Returns true if the editor with matching id and asset type exists
        /// @param inventory_id Inventory id.
        /// @param asset_type Asset type.
        /// @return True if the editor with matching id and asset type exists, false otherwise.
        bool Exists(const QString &inventory_id, asset_type_t asset_type) const;

        /// Deletes editor.
        /// @param inventory_id Inventory id.
        /// @return True if editor was found and deleted succesfully, false otherwise.
        bool Delete(const QString &inventory_id, asset_type_t asset_type);

        /// Deletes all editors.
        /// @param delete_later Sert true if you want to use QWidget::deleteLater instead of normal delete.
        void DeleteAll(bool delete_later = false);

    private:
        /// Returns editor object and removes it from the editor map.
        /// @param inventory_id Inventory id.
        /// @param asset_type Asset type.
        /// @return Editor object or null if not found.
        QWidget *TakeEditor(const QString &inventory_id, asset_type_t asset_type);

        /// Map of currently existing editors.
        EditorMap editors_;
    };
}

#endif

// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file   EditorManager.cpp
 *  @brief  EditorManager manages the editor windows.
 */

#include "StableHeaders.h"
#include "EditorManager.h"

namespace OgreAssetEditor
{

EditorManager::EditorManager()
{
}

EditorManager::~EditorManager()
{
    DeleteAll();
}

bool EditorManager::Add(const QString &inventory_id, RexTypes::asset_type_t asset_type, QObject *editor)
{
    if (!editor || inventory_id.isEmpty() || asset_type == RexTypes::RexAT_None)
        return false;

    editors_[qMakePair(inventory_id, asset_type)] = editor;
    return true;
}

QObject *EditorManager::GetEditor(const QString &inventory_id, RexTypes::asset_type_t asset_type)
{
    EditorMapKey key = qMakePair(inventory_id, asset_type);
    EditorMapIter it = editors_.find(key);
    if (it != editors_.end())
        return it.value();
    else
        return 0;
}

bool EditorManager::Exists(const QString &inventory_id, RexTypes::asset_type_t asset_type) const
{
    EditorMapKey key = qMakePair(inventory_id, asset_type);
    return editors_.contains(key);
}

bool EditorManager::Delete(const QString &inventory_id, RexTypes::asset_type_t asset_type)
{
    QObject *editor = TakeEditor(inventory_id, asset_type);
    if (editor)
    {
        SAFE_DELETE(editor);
        return true;
    }
    else
        return false;
}

void EditorManager::DeleteAll()
{
    MutableEditorMapIter it(editors_);
    while(it.hasNext())
    {
        QObject *editor = it.next().value();
        SAFE_DELETE(editor);
        it.remove();
    }
}

QObject *EditorManager::TakeEditor(const QString &inventory_id, RexTypes::asset_type_t asset_type)
{
    EditorMapKey key = qMakePair(inventory_id, asset_type);
    EditorMapIter it = editors_.find(key);
    if (it != editors_.end())
        return editors_.take(key);
    else
        return 0;
}

}

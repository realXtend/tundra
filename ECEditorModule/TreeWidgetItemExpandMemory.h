/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   TreeWidgetItemExpandMemory.h
 *  @brief  Utility class for keeping track of expanded items at a tree widget.
 */

#ifndef incl_ECEditorModule_TreeWidgetItemExpandMemory_h
#define incl_ECEditorModule_TreeWidgetItemExpandMemory_h

#include "ConfigurationManager.h"

#include <QObject>
#include <QSet>

class QTreeWidget;
class QTreeWidgetItem;

/// Utility class for keeping track of expanded items at a tree widget.
/** @todo Usage example
*/
class TreeWidgetItemExpandMemory : public QObject
{
    Q_OBJECT

public:
    /// Constructs the object and loads information about expanded items from config file.
    /** @param group Group name identifier.
        @param mgr Config manager.
    */
    TreeWidgetItemExpandMemory(const char *, const Foundation::ConfigurationManager &mgr);

    /// Destroyes the object and saves information about currently expanded items to config file.
    ~TreeWidgetItemExpandMemory();

    /// Expands... Collapses the item if it's identifier is not found in the item info set.
    /** @param treeWidet
        @param item
    */
    void ExpandItem(QTreeWidget *treeWidget, QTreeWidgetItem *item) const;

    ///
    /** @param item
    */
    QString GetIndentifierText(QTreeWidgetItem *item) const;

    /// Loads information about expanded items from config file.
    void Load();

    /// Saves information about currently expanded items to config file.
    void Save();

public slots:
    ///
    /** @param item
    */
    void HandleItemExpanded(QTreeWidgetItem *item);

    ///
    /** @param item
    */
    void HandleItemCollapsed(QTreeWidgetItem *item);

private:
    /// Returns information about expanded items as one string suitable to be saved to config file.
    std::string ToString() const;

    QSet<QString> items; ///< 
    Foundation::ConfigurationManager cfgMgr; ///< Config manager.
    std::string groupName; ///< Setting group name.
};

#endif

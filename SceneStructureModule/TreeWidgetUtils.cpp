/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   TreeWidgetUtils.cpp
 *  @brief  Tree widget utility functions.
 */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "MemoryLeakCheck.h"

void TreeWidgetSearch(QTreeWidget *treeWidget, int column, const QString &filter)
{
    QString f = filter.trimmed();
    bool expand = f.size() >= 3;
    QSet<QTreeWidgetItem *> alreadySetVisible;

    QTreeWidgetItemIterator it(treeWidget);
    while(*it)
    {
        QTreeWidgetItem *item = *it;
        if (!alreadySetVisible.contains(item))
        {
            if (f.isEmpty())
            {
                item->setHidden(false);
            }
            else if (item->text(column).contains(filter, Qt::CaseInsensitive))
            {
                item->setHidden(false);
                alreadySetVisible.insert(item);
                if (expand)
                    item->setExpanded(expand);

                // Make sure that all the parent items are visible too
                QTreeWidgetItem *parent = 0, *child = item;
                while((parent = child->parent()) != 0)
                {
                    parent->setHidden(false);
                    alreadySetVisible.insert(parent);
                    if (expand)
                        parent->setExpanded(expand);
                    child = parent;
                }
            }
            else
            {
                item->setHidden(true);
            }
        }

        ++it;
    }
}

void TreeWidgetExpandOrCollapseAll(QTreeWidget *treeWidget)
{
    bool expand = true;
    for (int i = 0; i < treeWidget->topLevelItemCount(); ++i)
    {
        QTreeWidgetItem *item = treeWidget->topLevelItem(i);
        if (item->childCount() >= 1 && item->isExpanded())
        {
            expand = false;
            break;
        }
    }

    if (expand)
        treeWidget->expandAll();
    else
        treeWidget->collapseAll();
}

void TreeWidgetSetCheckStateForAllItems(QTreeWidget *treeWidget, int column, Qt::CheckState state)
{
    treeWidget->setSortingEnabled(false);
    treeWidget->setUpdatesEnabled(false);

    QTreeWidgetItemIterator it(treeWidget);
    while(*it)
    {
        (*it)->setCheckState(column, state);
        ++it;
    }

    treeWidget->setSortingEnabled(true);
    treeWidget->setUpdatesEnabled(true);
}

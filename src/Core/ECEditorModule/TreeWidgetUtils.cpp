/**
    For conditions of distribution and use, see copyright notice in LICENSE

    @file   TreeWidgetUtils.cpp
    @brief  Tree widget utility functions. */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "TreeWidgetUtils.h"

#include "MemoryLeakCheck.h"

namespace
{
    void TreeWidgetSearchHelper(QTreeWidgetItem *root, QSet<QTreeWidgetItem*> &handled, QSet<QTreeWidgetItem*> &matched,
        bool negation, int column, const QString &filter, bool exactMatch, Qt::CaseSensitivity sensitivity, bool expandItems)
    {
        for (int i=0; i<root->childCount(); ++i)
        {
            QTreeWidgetItem *item = root->child(i);
            if (item && (!exactMatch && item->text(column).contains(filter, sensitivity)) ||
                (exactMatch && item->text(column).compare(filter, sensitivity) == 0))
            {
                if (!item->isDisabled())
                    item->setHidden(negation);
                matched << item;

                if (expandItems && !item->isHidden())
                    item->setExpanded(true);

                // Make sure that all the parent items are visible too
                QTreeWidgetItem *parent = 0, *child = item;
                while((parent = child->parent()) != 0)
                {
                    if (!item->isDisabled())
                        parent->setHidden(negation);
                    handled << parent;
                    if (expandItems && !parent->isHidden())
                        parent->setExpanded(true);
                    child = parent;
                }
            }
            else
                TreeWidgetSearchHelper(item, handled, matched, negation, column, filter, exactMatch, sensitivity, expandItems);
        }
    }
}

QSet<QTreeWidgetItem*> TreeWidgetSearch(QTreeWidget *treeWidget, int column, const QString &filter,
    bool exactMatch, Qt::CaseSensitivity sensitivity, bool expandItems)
{
    return TreeWidgetSearch(treeWidget, column, QStringList() << filter, exactMatch, sensitivity, expandItems);
}

QSet<QTreeWidgetItem*> TreeWidgetSearch(QTreeWidget *treeWidget, int column, const QStringList &filters,
    bool exactMatch, Qt::CaseSensitivity sensitivity, bool expandItems)
{
    QSet<QTreeWidgetItem *> handled;
    QSet<QTreeWidgetItem *> matched;
    
    foreach(QString filter, filters)
    {
        filter = filter.trimmed();

        // Negation search?
        const bool negation = (!filter.isEmpty() && filter[0] == '!');
        if (negation)
            filter = filter.mid(1);

        QTreeWidgetItemIterator it(treeWidget);
        while(*it)
        {
            QTreeWidgetItem *item = *it;

            // This item has been already handled with a previous filter
            // but we need to check its children matching this filter.
            if (handled.contains(item))
            {
                if (!filter.isEmpty())
                    TreeWidgetSearchHelper(item, handled, matched, negation, column, filter, exactMatch, sensitivity, expandItems);
                continue;
            }

            if (filter.isEmpty())
            {
                if (!item->isDisabled())
                    item->setHidden(false); // No filter, show everything.
            }
            else if ((!exactMatch && item->text(column).contains(filter, sensitivity)) ||
                (exactMatch && item->text(column).compare(filter, sensitivity) == 0))
            {
                // Hit with filter to column text
                if (!item->isDisabled())
                    item->setHidden(negation);
                handled << item;
                matched << item;

                if (expandItems && !item->isHidden())
                    item->setExpanded(true);

                // Make sure that all the parent items are visible too
                QTreeWidgetItem *parent = 0, *child = item;
                while((parent = child->parent()) != 0)
                {
                    if (!item->isDisabled())
                        parent->setHidden(negation);
                    handled << parent;
                    if (expandItems && !parent->isHidden())
                        parent->setExpanded(true);
                    child = parent;
                }
            }
            else // No hit
            {
                if (!item->isDisabled())
                    item->setHidden(!negation);
            }

            ++it;
        }
    }
    
    return matched;
}

bool TreeWidgetExpandOrCollapseAll(QTreeWidget *treeWidget)
{
    bool expand = true;
    for(int i = 0; i < treeWidget->topLevelItemCount(); ++i)
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
    return expand;
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

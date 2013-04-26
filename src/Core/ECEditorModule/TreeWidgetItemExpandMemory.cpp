/**
    For conditions of distribution and use, see copyright notice in LICENSE

    @file   TreeWidgetItemExpandMemory.cpp
    @brief  Utility class for keeping track of expanded items at a tree widget. */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "TreeWidgetItemExpandMemory.h"

#include "Framework.h"
#include "ConfigAPI.h"

#include <QTreeWidget>

#include "MemoryLeakCheck.h"

TreeWidgetItemExpandMemory::TreeWidgetItemExpandMemory(const char *group, Framework *fw) :
    framework(fw),
    groupName(group)
{
    Load();
}

TreeWidgetItemExpandMemory::~TreeWidgetItemExpandMemory()
{
    Save();
}

void TreeWidgetItemExpandMemory::ExpandItem(QTreeWidget *treeWidget, const QTreeWidgetItem *item) const
{
    if (!item)
        return;

    if (items.find(IndentifierForItem(item)) != items.end())
        treeWidget->expandItem(item);
    else
        treeWidget->collapseItem(item);

    for(int i = 0; i < item->childCount(); ++i)
        ExpandItem(treeWidget, item->child(i));
}

QString TreeWidgetItemExpandMemory::IndentifierForItem(const QTreeWidgetItem *item) const
{
    QList<QTreeWidgetItem *> items;
    const QTreeWidgetItem *c = item;
    QTreeWidgetItem *p = 0;
    while((p = c->parent()) != 0)
    {
        items.push_front(p);
        c = p;
    }

    QString indentifier;
    foreach(QTreeWidgetItem *i, items)
        indentifier.append(i->text(0) + ".");
    indentifier.append(item->text(0));

    return indentifier;
}

void TreeWidgetItemExpandMemory::Load()
{
    items.clear();
    QStringList setting = framework->Config()->Get("uimemory", groupName + " tree state", "expanded").toString().split("|");
    foreach(const QString &s, setting)
        if (!s.isEmpty())
            items.insert(s);
}

void TreeWidgetItemExpandMemory::Save()
{
    QString state = ToString();
    if (!state.isEmpty())
        framework->Config()->Set("uimemory", groupName + " tree state", "expanded", state);
}

void TreeWidgetItemExpandMemory::HandleItemExpanded(QTreeWidgetItem *item)
{
    QString idText = IndentifierForItem(item);
    int idx = idText.lastIndexOf('.');
    if (!idText.isEmpty() && (idx != idText.size() - 1))
        items.insert(idText);
}

void TreeWidgetItemExpandMemory::HandleItemCollapsed(QTreeWidgetItem *item)
{
    QString idText = IndentifierForItem(item);
    int idx = idText.lastIndexOf('.');
    if (!idText.isEmpty() && (idx != idText.size() - 1))
        items.remove(idText);
}

QString TreeWidgetItemExpandMemory::ToString() const
{
    QString ret;
    int idx = 0;
    foreach(const QString &item, items)
    {
        ret.append(item);
        if (idx < items.size() - 1)
            ret.append("|");
        ++idx;
    }

    return ret;
}

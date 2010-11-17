/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   AddContentWindow.cpp
 *  @brief  Window for adding new content and assets.
 */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "AddContentWindow.h"

#include "SceneDesc.h"

#include "MemoryLeakCheck.h"

AddContentWindow::AddContentWindow(QWidget *parent) : QWidget(parent)
{
    setWindowModality(Qt::ApplicationModal/*Qt::WindowModal*/);
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(tr("Add Content"));
    resize(400,500);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(5, 5, 5, 5);
    setLayout(layout);

    QLabel *entityLabel = new QLabel("The following entities will be created:");
    entityTreeWidget = new QTreeWidget;
    entityTreeWidget->setHeaderHidden(true);
    QPushButton *selectAllEntitiesButton = new QPushButton(tr("Select All"));
    QPushButton *deselectAllEntitiesButton = new QPushButton(tr("Deselect All"));
    QHBoxLayout *entityButtonsLayout = new QHBoxLayout;
    QSpacerItem *entityButtonSpacer = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    layout->addWidget(entityLabel);
    layout->addWidget(entityTreeWidget);
    entityButtonsLayout->addWidget(selectAllEntitiesButton);
    entityButtonsLayout->addWidget(deselectAllEntitiesButton);
    entityButtonsLayout->addSpacerItem(entityButtonSpacer);
    layout->insertLayout(-1, entityButtonsLayout);

    QLabel *assetLabel = new QLabel("The following assets will be uploaded:");
    assetTreeWidget = new QTreeWidget;
    assetTreeWidget->setHeaderHidden(true);
    QPushButton *selectAllAssetsButton = new QPushButton(tr("Select All"));
    QPushButton *deselectAllAssetsButton = new QPushButton(tr("Deselect All"));
    QHBoxLayout *assetButtonsLayout = new QHBoxLayout;
    QSpacerItem *assetButtonSpacer = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    QLabel *storageLabel = new QLabel("Asset storage:");
    QComboBox *storageComboBox = new QComboBox;
    storageComboBox->addItem("knet://127.0.0.1:2345");

    layout->addWidget(assetLabel);
    layout->addWidget(assetTreeWidget);
    assetButtonsLayout->addWidget(selectAllAssetsButton);
    assetButtonsLayout->addWidget(deselectAllAssetsButton);
    assetButtonsLayout->addSpacerItem(assetButtonSpacer);
    assetButtonsLayout->addWidget(storageLabel);
    assetButtonsLayout->addWidget(storageComboBox);
    layout->insertLayout(-1, assetButtonsLayout);

    QPushButton *addContentButton = new QPushButton(tr("Add content"));
    QPushButton *cancelButton = new QPushButton(tr("Cancel"));

    QHBoxLayout *buttonsLayout = new QHBoxLayout;
    QSpacerItem *buttonSpacer = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    buttonsLayout->addSpacerItem(buttonSpacer);
    buttonsLayout->addWidget(addContentButton);
    buttonsLayout->addWidget(cancelButton);
    layout->insertLayout(-1, buttonsLayout);

    connect(addContentButton, SIGNAL(clicked()), SLOT(AddContent()));
    connect(cancelButton, SIGNAL(clicked()), SLOT(Close()));
    connect(selectAllEntitiesButton, SIGNAL(clicked()), SLOT(SelectAllEntities()));
    connect(deselectAllEntitiesButton, SIGNAL(clicked()), SLOT(DeselectAllEntities()));
    connect(selectAllAssetsButton, SIGNAL(clicked()), SLOT(SelectAllAssets()));
    connect(deselectAllAssetsButton, SIGNAL(clicked()), SLOT(DeselectAllAssets()));
}

AddContentWindow::~AddContentWindow()
{
    QTreeWidgetItemIterator it(entityTreeWidget);
    while(*it)
    {
        QTreeWidgetItem *item = *it;
        SAFE_DELETE(item);
        ++it;
    }

    QTreeWidgetItemIterator it2(assetTreeWidget);
    while(*it2)
    {
        QTreeWidgetItem *item = *it2;
        SAFE_DELETE(item);
        ++it2;
    }
}

void AddContentWindow::AddDescription(const SceneDesc &desc)
{
    sceneDesc = desc;
    std::set<AttributeDesc> assetRefs;

    foreach(EntityDesc e, desc.entities)
    {
        QTreeWidgetItem *eItem = new QTreeWidgetItem;
        eItem->setText(0, e.id + " " + e.name);
        eItem->setCheckState(0, Qt::Checked);
        entityTreeWidget->addTopLevelItem(eItem);

        foreach(ComponentDesc c, e.components)
        {
            // Not showing components in the UI for now.
            /*
            QTreeWidgetItem *cItem = new QTreeWidgetItem;
            cItem->setText(0, c.typeName + " " + c.name);
            eItem->addChild(cItem);
            */

            // Gather asset references. They're shown in their own tree widget.
            foreach(AttributeDesc a, c.attributes)
                if (a.typeName == "assetreference")
                    assetRefs.insert(a);
        }
    }

    // Add asset references. Do not show duplicates.
    foreach(AttributeDesc a, assetRefs)
    {
        QTreeWidgetItem *aItem = new QTreeWidgetItem;
        aItem->setText(0, a.name + ": " + a.value);
        aItem->setCheckState(0, Qt::Checked);
        assetTreeWidget->addTopLevelItem(aItem);
    }
}

void AddContentWindow::SelectAllEntities()
{
    QTreeWidgetItemIterator it(entityTreeWidget);
    while(*it)
    {
        (*it)->setCheckState(0, Qt::Checked);
        ++it;
    }
}

void AddContentWindow::DeselectAllEntities()
{
    QTreeWidgetItemIterator it(entityTreeWidget);
    while(*it)
    {
        (*it)->setCheckState(0, Qt::Unchecked);
        ++it;
    }
}

void AddContentWindow::SelectAllAssets()
{
    QTreeWidgetItemIterator it(assetTreeWidget);
    while(*it)
    {
        (*it)->setCheckState(0, Qt::Checked);
        ++it;
    }
}

void AddContentWindow::DeselectAllAssets()
{
    QTreeWidgetItemIterator it(assetTreeWidget);
    while(*it)
    {
        (*it)->setCheckState(0, Qt::Unchecked);
        ++it;
    }
}

void AddContentWindow::AddContent()
{
    SceneDesc newDesc = sceneDesc;
    QTreeWidgetItemIterator it(entityTreeWidget);
    while(*it)
    {
        if ((*it)->checkState(0) == Qt::Unchecked)
        {
            QStringList params = (*it)->text(0).split(" ");
            EntityDesc ed = { params[0], params.size() > 1 ? params[1] : "" };
            QList<EntityDesc>::const_iterator eit = qFind(newDesc.entities, ed);
            if (eit != newDesc.entities.end())
                newDesc.entities.removeOne(*eit);
        }

        ++it;
    }

    foreach(EntityDesc ed, newDesc.entities)
        std::cout << ed.id.toStdString() << " " << ed.name.toStdString() << std::endl;

    //close();
}

void AddContentWindow::Close()
{
    close();
}


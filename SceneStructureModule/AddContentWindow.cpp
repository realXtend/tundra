/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   AddContentWindow.cpp
 *  @brief  Window for adding new content and assets.
 */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "AddContentWindow.h"
#include "SceneStructureModule.h"

#include "SceneDesc.h"
#include "SceneManager.h"
#include "SceneImporter.h"
#include "LoggingFunctions.h"

DEFINE_POCO_LOGGING_FUNCTIONS("AddContentWindow")

#include "MemoryLeakCheck.h"

class EntityWidgetItem : public QTreeWidgetItem
{
public:
    explicit EntityWidgetItem(const EntityDesc &edesc) : desc(edesc)
    {
        setText(0, desc.id.isEmpty() ? desc.name : desc.id + ": " + desc.name);
        setCheckState(0, Qt::Checked);
    }
    EntityDesc desc;
};

class AssetWidgetItem : public QTreeWidgetItem
{
public:
    explicit AssetWidgetItem(const AttributeDesc &adesc) : desc(adesc)
    {
        setText(0, desc.name + ": " + desc.value);
        setCheckState(0, Qt::Checked);
    }
    AttributeDesc desc;
};

AddContentWindow::AddContentWindow(const Scene::ScenePtr &dest, QWidget *parent) :
    QWidget(parent),
    scene(dest)
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
    storageComboBox = new QComboBox;
    storageComboBox->addItem("Local");
    storageComboBox->setDisabled(true);

    layout->addWidget(assetLabel);
    layout->addWidget(assetTreeWidget);
    assetButtonsLayout->addWidget(selectAllAssetsButton);
    assetButtonsLayout->addWidget(deselectAllAssetsButton);
    assetButtonsLayout->addSpacerItem(assetButtonSpacer);
    assetButtonsLayout->addWidget(storageLabel);
    assetButtonsLayout->addWidget(storageComboBox);
    layout->insertLayout(-1, assetButtonsLayout);

    addContentButton = new QPushButton(tr("Add content"));
    cancelButton = new QPushButton(tr("Cancel"));

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
        EntityWidgetItem *eItem = new EntityWidgetItem(e);
        entityTreeWidget->addTopLevelItem(eItem);

        foreach(ComponentDesc c, e.components)
        {
            // Not showing components in the UI for now.
            /*
            QTreeWidgetItem *cItem = new QTreeWidgetItem;
            cItem->setText(0, c.typeName + " " + c.name);
            eItem->addChild(cItem);
            */

            // Gather non-empty asset references. They're shown in their own tree widget.
            foreach(AttributeDesc a, c.attributes)
                if (a.typeName == "assetreference" && !a.value.isEmpty())
                    assetRefs.insert(a);
        }
    }

    // Add asset references. Do not show duplicates.
    foreach(AttributeDesc a, assetRefs)
    {
        AssetWidgetItem *aItem = new AssetWidgetItem(a);
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
    QTreeWidgetItemIterator eit(entityTreeWidget);
    while(*eit)
    {
        EntityWidgetItem *eitem = dynamic_cast<EntityWidgetItem *>(*eit);
        if (eitem && eitem->checkState(0) == Qt::Unchecked)
        {
            QList<EntityDesc>::const_iterator eit = qFind(newDesc.entities, eitem->desc);
            if (eit != newDesc.entities.end())
                newDesc.entities.removeOne(*eit);
        }

        ++eit;
    }

    QTreeWidgetItemIterator ait(assetTreeWidget);
    while(*ait)
    {
        AssetWidgetItem *aitem = dynamic_cast<AssetWidgetItem *>(*ait);
        if (aitem && aitem->checkState(0) == Qt::Unchecked)
            for(int i = 0; i < newDesc.entities.size(); ++i)
                for(int j = 0; j < newDesc.entities[i].components.size(); ++j)
                {
                    QMutableListIterator<AttributeDesc> it(newDesc.entities[i].components[j].attributes);
                    while(it.hasNext())
                        if (it.next() == aitem->desc)
                            it.remove();
                }

        ++ait;
    }
/*
    foreach(EntityDesc ed, newDesc.entities)
    {
        std::cout << ed.id.toStdString() << " " << ed.name.toStdString() << std::endl;
        foreach(ComponentDesc cd, ed.components)
            foreach(AttributeDesc ad, cd.attributes)
                std::cout << ad.name.toStdString() << " " << ad.value.toStdString() << std::endl;
    }
*/
    // get asset storage storageComboBox->
    // rewrite asset refs.

    Scene::ScenePtr destScene = scene.lock();
    if (!destScene)
        return;

    QList<Scene::Entity *> entities;

    switch(newDesc.type)
    {
    case SceneDesc::Naali:
        entities = destScene->CreateContentFromSceneDescription(newDesc, false, AttributeChange::Default);
        break;
    case SceneDesc::OgreMesh:
    {
        boost::filesystem::path path(newDesc.filename.toStdString());
        std::string dirname = path.branch_path().string();

        TundraLogic::SceneImporter importer(destScene);
        Scene::EntityPtr entity = importer.ImportMesh(newDesc.filename.toStdString(), dirname, "./data/assets",
            Transform(),std::string(), AttributeChange::Default, true, true, std::string(), newDesc);
        if (entity)
            entities << entity.get();
        break;
    }
    case SceneDesc::OgreScene:
    {
        boost::filesystem::path path(newDesc.filename.toStdString());
        std::string dirname = path.branch_path().string();

        TundraLogic::SceneImporter importer(destScene);
        entities = importer.Import(newDesc.filename.toStdString(), dirname, "./data/assets", Transform(),
            AttributeChange::Default, false/*clearScene*/, true, false, newDesc);
    }
    default:
        LogError("Invalid scene description type.");
        break;
    }

    if (entities.size())
    {
        if (position != Vector3df())
            SceneStructureModule::CentralizeEntitiesTo(position, entities);

        addContentButton->setEnabled(false);
        cancelButton->setText(tr("Close"));
    }
}

void AddContentWindow::Close()
{
    close();
}


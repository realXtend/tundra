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

#include "Framework.h"
#include "AssetAPI.h"
#include "IAssetStorage.h"
#include "IAssetUploadTransfer.h"
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

/*class AssetWidgetItem : public QTreeWidgetItem
{
public:
    explicit AssetWidgetItem(const AttributeDesc &adesc) : desc(adesc)
    {
        setText(0, desc.name + ": " + desc.value);
        setCheckState(0, Qt::Checked);
    }
    AttributeDesc desc;
};*/

class AssetWidgetItem : public QTreeWidgetItem
{
public:
    explicit AssetWidgetItem(const AssetDesc &adesc) : desc(adesc)
    {
        setCheckState(0, Qt::Checked);
        setText(1, desc.typeName);
        setText(2, desc.filename);
        setText(3, desc.destinationName);
        setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
    }
    AssetDesc desc;
};

AddContentWindow::AddContentWindow(Foundation::Framework *fw, const Scene::ScenePtr &dest, QWidget *parent) :
    QWidget(parent),
    framework(fw),
    scene(dest)
{
    setWindowModality(Qt::ApplicationModal/*Qt::WindowModal*/);
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(tr("Add Content"));
    resize(600,500);

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
    assetTreeWidget->setColumnCount(4);
    QStringList labels(QStringList() << tr("Upload") << tr("Type") << tr("Source name") << tr("Destination name"));
    assetTreeWidget->setHeaderLabels(labels);
//    assetTreeWidget->header()->setResizeMode(QHeaderView::ResizeToContents);
    QPushButton *selectAllAssetsButton = new QPushButton(tr("Select All"));
    QPushButton *deselectAllAssetsButton = new QPushButton(tr("Deselect All"));
    QHBoxLayout *assetButtonsLayout = new QHBoxLayout;
    QSpacerItem *assetButtonSpacer = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    QLabel *storageLabel = new QLabel("Asset storage:");
    storageComboBox = new QComboBox;
    // Get available asset storages.
    foreach(AssetStoragePtr storage, framework->Asset()->GetAssetStorages())
        storageComboBox->addItem(storage->Name());

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
            /*
            foreach(AttributeDesc a, c.attributes)
                if (a.typeName == "assetreference" && !a.value.isEmpty())
                    assetRefs.insert(a);
            */
        }
    }

    // Add asset references. Do not show duplicates.
    /*
    foreach(AttributeDesc a, assetRefs)
    {
        AssetWidgetItem *aItem = new AssetWidgetItem(a);
        assetTreeWidget->addTopLevelItem(aItem);
    }
    */

    // Add asset references. Do not show duplicates.
    std::set<AssetDesc> assets;
    foreach(AssetDesc a, desc.assets)
        assets.insert(a);

    foreach(AssetDesc a, assets)
    {
        AssetWidgetItem *aItem = new AssetWidgetItem(a);
        assetTreeWidget->addTopLevelItem(aItem);
        if ((a.typeName == "material" && a.data.isEmpty()) || (a.typeName != "material" && !QFile::exists(a.filename)))
        {
            aItem->setBackgroundColor(2, Qt::red);
            aItem->setCheckState(0, Qt::Unchecked);
        }
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
    AssetStoragePtr dest = framework->Asset()->GetAssetStorage(storageComboBox->currentText());
    if (!dest)
    {
        LogError("Could not retrieve asset storage " + storageComboBox->currentText().toStdString() + ".");
        return;
    }

    // Filter which entities will be created and which assets will be uploaded.
    SceneDesc newDesc = sceneDesc;
    QTreeWidgetItemIterator eit(entityTreeWidget);
    while(*eit)
    {
        EntityWidgetItem *eitem = dynamic_cast<EntityWidgetItem *>(*eit);
        if (eitem && eitem->checkState(0) == Qt::Unchecked)
        {
            QList<EntityDesc>::const_iterator ei = qFind(newDesc.entities, eitem->desc);
            if (ei != newDesc.entities.end())
                newDesc.entities.removeOne(*ei);
        }

        ++eit;
    }

    QTreeWidgetItemIterator ait(assetTreeWidget);
    while(*ait)
    {
        AssetWidgetItem *aitem = dynamic_cast<AssetWidgetItem *>(*eit);
        if (aitem  && aitem ->checkState(0) == Qt::Unchecked)
        {
            QList<AssetDesc>::const_iterator ai = qFind(newDesc.assets, aitem->desc);
            if (ai != newDesc.assets.end())
                newDesc.assets.removeOne(*ai);
        }

        ++ait;
    }

    // Rewrite asset refs
    QMutableListIterator<AssetDesc> rewriteIt(newDesc.assets);
    while(rewriteIt.hasNext())
    {
        QString destName = rewriteIt.next().destinationName;
        rewriteIt.value().destinationName = dest->GetFullAssetURL(destName);
    }

    Scene::ScenePtr destScene = scene.lock();
    if (!destScene)
        return;

    // Create entities.
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

    if (!newDesc.assets.empty())
        LogDebug("Starting uploading of " + ToString(newDesc.assets.size()) + " asset(s).");

    foreach(AssetDesc ad, newDesc.assets)
    {
        try
        {
            IAssetUploadTransfer *transfer = 0;

            QString assetName;

            if (!ad.filename.isEmpty() && ad.data.isEmpty())
            {
//                LogDebug("Starting upload of ."+ ad.filename.toStdString());
                transfer = framework->Asset()->UploadAssetFromFile(ad.filename.toStdString().c_str(),
                    dest, ad.destinationName.toStdString().c_str());
            }
            else if (/*ad.filename.isEmpty() && */!ad.data.isEmpty())
            {
//                LogDebug("Starting upload of ."+ ad.destinationName.toStdString());
                transfer = framework->Asset()->UploadAssetFromFileInMemory((const u8*)QString(ad.data).toStdString().c_str(),
                    ad.data.size(), dest, ad.destinationName.toStdString().c_str());
            }
            else
                LogError("Could not upload.");

            if (transfer)
            {
                connect(transfer, SIGNAL(Completed(IAssetUploadTransfer *)), SLOT(HandleUploadCompleted(IAssetUploadTransfer *)));
                connect(transfer, SIGNAL(Failed(IAssetUploadTransfer *)), SLOT(HandleUploadFailed(IAssetUploadTransfer *)));
            }
        }
        catch(const Exception &e)
        {
            LogError(std::string(e.what()));
        }
    }

    if (entities.size() || newDesc.assets.size())
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

void AddContentWindow::HandleUploadCompleted(IAssetUploadTransfer *transfer)
{
    assert(transfer);
//    LogDebug("Upload completed, " + transfer->sourceFilename.toStdString() + " -> " + transfer->destinationName.toStdString());
}

void AddContentWindow::HandleUploadFailed(IAssetUploadTransfer *transfer)
{
    assert(transfer);
//    LogDebug("Upload failed for " + transfer->sourceFilename.toStdString() + "/" + transfer->destinationName.toStdString());
}


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
#include "TreeWidgetUtils.h"

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

namespace fs = boost::filesystem;

typedef QMap<QString, QString> RefMap;

void ReplaceReferences(QByteArray &material, const RefMap &refs)
{
    QString matString(material);
    QStringList lines = matString.split("\n");
    for(int i = 0; i < lines.size(); ++i)
    {
        int idx = lines[i].indexOf("texture ");
        if (idx != -1)
        {
            QString texName = lines[i].mid(idx + 8).trimmed();
            texName = AssetAPI::ExtractFilenameFromAssetRef(texName); ///\todo This line is wrong and should be removed.
            // There is a problem in the refmap that it doesn't store the original assetrefs, but stores the filenames
            // without paths. Therefore we need to do the comparison here also without paths.
            RefMap::const_iterator it = refs.find(texName);
            if (it != refs.end())
            {
                lines[i] = "texture " + it.value();
                for(int spaceIdx = 0; spaceIdx < idx; ++spaceIdx)
                    lines[i].prepend(" ");
            }
        }
    }

    material = lines.join("\n").toAscii();
}

// Entity tree widget column index enumeration.
const int cColumnEntityCreate = 0; ///< Create column index.
const int cColumnEntityId = 1; ///< ID column index.
const int cColumnEntityName = 2; ///< Name column index.

// Asset tree widget column index enumeration.
const int cColumnAssetUpload = 0; ///< Upload column index;
const int cColumnAssetTypeName = 1; ///< Type name column index.
const int cColumnAssetSourceName = 2; ///< Source name column index.
const int cColumnAssetSubname = 3; ///< Subname column index.
const int cColumnAssetDestName = 4; ///< Destination name column index.

/// Tree widget item representing an entity.
class EntityWidgetItem : public QTreeWidgetItem
{
public:
    /// Constructor.
    /** @param edesc Entity description.
    */
    explicit EntityWidgetItem(const EntityDesc &edesc) : desc(edesc)
    {
        setCheckState(cColumnEntityCreate, Qt::Checked);
        setText(cColumnEntityId, desc.id);
        setText(cColumnEntityName, desc.name);
    }

    /// QTreeWidgetItem override. Peforms case-insensitive comparison.
    bool operator <(const QTreeWidgetItem &rhs) const
    {
        int column = treeWidget()->sortColumn();
        if (column == cColumnEntityCreate)
            return checkState(column) < rhs.checkState(column);
        else if (column == cColumnEntityId)
            return text(column).toInt() < rhs.text(column).toInt();
        else
            return text(column).toLower() < rhs.text(column).toLower();
    }

    EntityDesc desc; ///< Entity description of the item.
};

/// Tree widget item representing an asset.
class AssetWidgetItem : public QTreeWidgetItem
{
public:
    /// Constructor.
    /** @param adesc Asset description.
    */
    explicit AssetWidgetItem(const AssetDesc &adesc) : desc(adesc)
    {
        RewriteText();
    }

    /// QTreeWidgetItem override. Peforms case-insensitive comparison.
    bool operator <(const QTreeWidgetItem &rhs) const
    {
        int column = treeWidget()->sortColumn();
        if (column == cColumnAssetUpload)
            return checkState(column) < rhs.checkState(column);
        else
            return text(column).toLower() < rhs.text(column).toLower();
    }

    /// Rewrites the items visible text accordingly to the asset description the item owns.
    void RewriteText()
    {
        setCheckState(cColumnAssetUpload, Qt::Checked);
        setText(cColumnAssetTypeName, desc.typeName);
        setText(cColumnAssetSourceName, desc.source);
        setText(cColumnAssetSubname, desc.subname);
        setText(cColumnAssetDestName, desc.destinationName);
    }

    AssetDesc desc; ///< Asset description of the item.
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

    QLabel *entityLabel = new QLabel(tr("The following entities will be created:"));

    entityTreeWidget = new QTreeWidget;
    entityTreeWidget->setColumnCount(3);
    entityTreeWidget->setHeaderLabels(QStringList(QStringList() << tr("Create") << tr("ID") << tr("Name")));
    entityTreeWidget->header()->setResizeMode(QHeaderView::ResizeToContents);

    QPushButton *selectAllEntitiesButton = new QPushButton(tr("Select All"));
    QPushButton *deselectAllEntitiesButton = new QPushButton(tr("Deselect All"));
    QHBoxLayout *entityButtonsLayout = new QHBoxLayout;
    QSpacerItem *entityButtonSpacer = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    QSpacerItem *middleSpacer = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Fixed);

    layout->addWidget(entityLabel);
    layout->addWidget(entityTreeWidget);
    entityButtonsLayout->addWidget(selectAllEntitiesButton);
    entityButtonsLayout->addWidget(deselectAllEntitiesButton);
    entityButtonsLayout->addSpacerItem(entityButtonSpacer);
    layout->insertLayout(-1, entityButtonsLayout);
    layout->insertSpacerItem(-1, middleSpacer);

    QLabel *assetLabel = new QLabel(tr("The following assets will be uploaded:"));

    assetTreeWidget = new QTreeWidget;
    assetTreeWidget->setColumnCount(5);
    QStringList labels;
    labels << tr("Upload") << tr("Type") << tr("Source name") << tr("Source subname") << tr("Destination name");
    assetTreeWidget->setHeaderLabels(labels);

    QPushButton *selectAllAssetsButton = new QPushButton(tr("Select All"));
    QPushButton *deselectAllAssetsButton = new QPushButton(tr("Deselect All"));
    QHBoxLayout *assetButtonsLayout = new QHBoxLayout;
    QSpacerItem *assetButtonSpacer = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    QLabel *storageLabel = new QLabel(tr("Asset storage:"));
    storageComboBox = new QComboBox;

    // Get available asset storages. Set default storage selected as default.
//    storageComboBox->addItem(tr("Don't adjust"), "DoNotAdjust");
    storageComboBox->addItem(tr("Default storage"), "DefaultStorage");
    AssetStoragePtr def = framework->Asset()->GetDefaultAssetStorage();
    std::vector<AssetStoragePtr> storages = framework->Asset()->GetAssetStorages();
    for(size_t i = 0; i < storages.size(); ++i)
    {
        storageComboBox->addItem(storages[i]->ToString(), storages[i]->Name());
        if (def && storages[i] == def)
        {
            QFont font = QApplication::font();
            font.setBold(true);
            storageComboBox->setItemData(i+1, font, Qt::FontRole); // i+1 as we already have the default storage item
            storageComboBox->setCurrentIndex(i+1);
        }
    }

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
    connect(assetTreeWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)),
        SLOT(CheckIfColumnIsEditable(QTreeWidgetItem *, int)));
    connect(storageComboBox, SIGNAL(currentIndexChanged(int)), SLOT(RewriteDestinationNames()));
}

AddContentWindow::~AddContentWindow()
{
    // Disable ResizeToContents, Qt goes sometimes into eternal loop after
    // ~AddContentWindow() if we have lots (hudreds or thousands) of items.
    entityTreeWidget->header()->setResizeMode(QHeaderView::Interactive);
    assetTreeWidget->header()->setResizeMode(QHeaderView::Interactive);

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

    AddEntities(desc.entities);
    AddAssets(desc.assets);
}

/*
void AddContentWindow::AddDescriptions(const QList<SceneDesc> &descs)
{
}
*/

void AddContentWindow::AddFiles(const QStringList &fileNames)
{
    assetTreeWidget->setSortingEnabled(false);

    SceneDesc desc;
    foreach(QString file, fileNames)
    {
        AssetDesc ad;
        ad.source = file;
        ad.dataInMemory = false;
        QString type = GetResourceTypeFromResourceFileName(file.toStdString().c_str());
        ad.typeName = type.isEmpty() ? "Binary" : type;
        ad.destinationName = fs::path(file.toStdString()).leaf().c_str();
        desc.assets[qMakePair(ad.source, ad.subname)]= ad;
    }

    sceneDesc = desc;

    AddAssets(desc.assets);
}

void AddContentWindow::AddEntities(const QList<EntityDesc> &entityDescs)
{
    // Disable sorting while we insert items.
    entityTreeWidget->setSortingEnabled(false);

    foreach(EntityDesc e, entityDescs)
    {
        EntityWidgetItem *eItem = new EntityWidgetItem(e);
        entityTreeWidget->addTopLevelItem(eItem);

        // Not showing components nor attributes in the UI for now.
/*
        foreach(ComponentDesc c, e.components)
        {
            QTreeWidgetItem *cItem = new QTreeWidgetItem;
            cItem->setText(0, c.typeName + " " + c.name);
            eItem->addChild(cItem);
*/

            // Gather non-empty asset references. They're shown in their own tree widget.
/*
            foreach(AttributeDesc a, c.attributes)
                if (a.typeName == "assetreference" && !a.value.isEmpty())
                    assetRefs.insert(a);
        }
*/
    }

    entityTreeWidget->setSortingEnabled(true);
}

void AddContentWindow::AddAssets(const SceneDesc::AssetMap &assetDescs)
{
    assetTreeWidget->setSortingEnabled(false);

    foreach(AssetDesc a, assetDescs)
    {
        AssetWidgetItem *aItem = new AssetWidgetItem(a);
        assetTreeWidget->addTopLevelItem(aItem);

        QString basePath(fs::path(sceneDesc.filename.toStdString()).branch_path().string().c_str());
        QString outFilePath;
        AssetAPI::FileQueryResult res = framework->Asset()->QueryFileLocation(a.source, basePath, outFilePath);
        /*if (res == AssetAPI::FileQueryLocalFileFound)
        {
            // If file is found locally rewrite the source for asset desc.
            QList<AssetDesc>::iterator ai = qFind(sceneDesc.assets.begin(), sceneDesc.assets.end(), aItem->desc);
            if (ai != sceneDesc.assets.end())
            {
                aItem->desc.source = outFilePath;
                aItem->RewriteText();
                (*ai).source = outFilePath;
            }
        }
        */
        if ((a.typeName == "material" && a.data.isEmpty()) || res == AssetAPI::FileQueryLocalFileMissing)
        {
            // File not found, mark the item red and disable it.
            aItem->setBackgroundColor(cColumnAssetSourceName, Qt::red);
            aItem->setCheckState(cColumnAssetUpload, Qt::Unchecked);
            aItem->setText(cColumnAssetDestName, "");
            aItem->setDisabled(true);
        }
        else if (res == AssetAPI::FileQueryExternalFile)
        {
            // External reference, mark the item gray and disable it.
            aItem->setBackgroundColor(cColumnAssetSourceName, Qt::gray);
            aItem->setCheckState(cColumnAssetUpload, Qt::Unchecked);
            aItem->setText(cColumnAssetDestName, "");
            aItem->setDisabled(true);
        }
    }

    RewriteDestinationNames();

    // Enable sorting, resize header sections to contents.
    assetTreeWidget->setSortingEnabled(true);
    assetTreeWidget->header()->resizeSections(QHeaderView::ResizeToContents);

    // Sort asset items initially so that erroneous are first
    assetTreeWidget->sortItems(cColumnAssetUpload, Qt::AscendingOrder);
}

void AddContentWindow::RewriteAssetReferences(SceneDesc &sceneDesc, const AssetStoragePtr &dest, bool useDefaultStorage)
{
//    QString path(fs::path(newDesc.filename.toStdString()).branch_path().string().c_str());

    QList<SceneDesc::AssetMapKey> keysWithSubname;
    foreach(SceneDesc::AssetMapKey key, sceneDesc.assets.keys())
        if (!key.second.isEmpty())
            keysWithSubname.append(key);

    QMutableListIterator<EntityDesc > edIt(sceneDesc.entities);
    while(edIt.hasNext())
    {
        QMutableListIterator<ComponentDesc> cdIt(edIt.next().components);
        while(cdIt.hasNext())
        {
            QMutableListIterator<AttributeDesc> adIt(cdIt.next().attributes);
            while(adIt.hasNext())
            {
                adIt.next();
                if (adIt.value().typeName == "assetreference" || adIt.value().typeName == "assetreferencelist")
                {
                    QStringList values = adIt.value().value.split(";");
                    QStringList newValues;
                    foreach(QString value, values)
                    {
                        QString subname;

                        if (!keysWithSubname.isEmpty())
                        {
                            ///\todo This string manipulation/crafting doesn't work for .zip files, only for materials and COLLADA files
                            int slashIdx = value.lastIndexOf("/");
                            int dotIdx = value.lastIndexOf(".");
                            QString str = value.mid(slashIdx + 1, dotIdx - slashIdx - 1);

                            foreach(SceneDesc::AssetMapKey key, keysWithSubname)
                                if (value == key.first && str == key.second)
                                {
                                    value = key.first;
                                    subname = key.second;
                                    break;
                                }
                        }

                        SceneDesc::AssetMapKey key = qMakePair(value, subname);
                        if (sceneDesc.assets.contains(key))
                            if (useDefaultStorage)
                                newValues << AssetAPI::ExtractFilenameFromAssetRef(sceneDesc.assets[key].destinationName);
                            else
                                newValues << dest->GetFullAssetURL(sceneDesc.assets[key].destinationName);
                    }

                    if (!newValues.isEmpty())
                        adIt.value().value = newValues.join(";");
                    // After the above lines the asset reference attribute descs do not point to the original source assets.
                }
            }
        }
    }
}

void AddContentWindow::SelectAllEntities()
{
    TreeWidgetSetCheckStateForAllItems(entityTreeWidget, cColumnEntityCreate, Qt::Checked);
}

void AddContentWindow::DeselectAllEntities()
{
    TreeWidgetSetCheckStateForAllItems(entityTreeWidget, cColumnEntityCreate, Qt::Unchecked);
}

void AddContentWindow::SelectAllAssets()
{
    TreeWidgetSetCheckStateForAllItems(assetTreeWidget, cColumnAssetUpload, Qt::Checked);
}

void AddContentWindow::DeselectAllAssets()
{
    TreeWidgetSetCheckStateForAllItems(assetTreeWidget, cColumnAssetUpload, Qt::Unchecked);
}

void AddContentWindow::AddContent()
{
    QString storageName = storageComboBox->itemData(storageComboBox->currentIndex()).toString();
    AssetStoragePtr dest;
    if (storageName == "DefaultStorage")
        dest = framework->Asset()->GetDefaultAssetStorage();
    else
        dest = framework->Asset()->GetAssetStorage(storageName);
    if (!dest)
    {
        LogError("Could not retrieve asset storage " + storageName.toStdString() + ".");

        // Regenerate storage combo box items to make sure that we're up-to-date.
        storageComboBox->clear();
        foreach(AssetStoragePtr storage, framework->Asset()->GetAssetStorages())
            storageComboBox->addItem(storage->ToString(), storage->Name());

        return;
    }

    // Filter which entities will be created and which assets will be uploaded.
    SceneDesc newDesc = sceneDesc;
    QTreeWidgetItemIterator eit(entityTreeWidget);
    while(*eit)
    {
        EntityWidgetItem *eitem = dynamic_cast<EntityWidgetItem *>(*eit);
        if (eitem && eitem->checkState(cColumnEntityCreate) == Qt::Unchecked)
        {
            QList<EntityDesc>::const_iterator ei = qFind(newDesc.entities, eitem->desc);
            if (ei != newDesc.entities.end())
                newDesc.entities.removeOne(*ei);
        }

        ++eit;
    }

    // Rewrite components' asset refs
    bool useDefault = storageName == "DefaultStorage";
    RewriteAssetReferences(newDesc, dest, useDefault);

    RefMap refs;
    QTreeWidgetItemIterator ait(assetTreeWidget);
    while(*ait)
    {
        AssetWidgetItem *aitem = dynamic_cast<AssetWidgetItem *>(*ait);
        assert(aitem);
        if (aitem)
        {
            if (aitem->checkState(cColumnAssetUpload) == Qt::Unchecked)
            {
                bool removed = newDesc.assets.remove(qMakePair(aitem->desc.source, aitem->desc.subname));
                if (!removed)
                    LogDebug("Couldn't find and remove " + aitem->desc.source.toStdString() + "from asset map.");
            }
            else
            {
                // Add textures to special map for later use.
                ///\todo This logic will be removed in the future, as we need it generic for any types of assets.
                if (aitem->desc.typeName == "texture")
                {
                    int idx = aitem->desc.source.lastIndexOf("/");
                    refs[aitem->desc.source.mid(idx != -1 ? idx + 1 : 0).trimmed()] = aitem->desc.destinationName;
                }
            }
        }

        ++ait;
    }

    // Rewrite asset refs
    QMutableMapIterator<SceneDesc::AssetMapKey, AssetDesc> rewriteIt(newDesc.assets);
    while(rewriteIt.hasNext())
    {
        rewriteIt.next();
        if (rewriteIt.value().typeName.contains("material", Qt::CaseInsensitive))
            ///\todo This logic will be removed in the future, as we need it generic for any types of assets.
            ReplaceReferences(rewriteIt.value().data, refs);
    }

    Scene::ScenePtr destScene = scene.lock();
    if (!destScene)
        return;

    // Upload
    if (!newDesc.assets.empty())
        LogDebug("Starting uploading of " + ToString(newDesc.assets.size()) + " asset" + "(s).");

    foreach(AssetDesc ad, newDesc.assets)
    {
        try
        {
            AssetUploadTransferPtr transfer;

            if (ad.dataInMemory)
            {
                transfer = framework->Asset()->UploadAssetFromFileInMemory((const u8*)QString(ad.data).toStdString().c_str(),
                    ad.data.size(), dest, ad.destinationName.toStdString().c_str());
            }
            else
            {
                transfer = framework->Asset()->UploadAssetFromFile(ad.source.toStdString().c_str(),
                    dest, ad.destinationName.toStdString().c_str());
            }

            if (transfer)
            {
                connect(transfer.get(), SIGNAL(Completed(IAssetUploadTransfer *)), SLOT(HandleUploadCompleted(IAssetUploadTransfer *)));
                connect(transfer.get(), SIGNAL(Failed(IAssetUploadTransfer *)), SLOT(HandleUploadFailed(IAssetUploadTransfer *)));
            }
        }
        catch(const Exception &e)
        {
            LogError(std::string(e.what()));
        }
    }

    // Create entities.
    QList<Scene::Entity *> entities;
    switch(newDesc.type)
    {
    case SceneDesc::Naali:
    case SceneDesc::OgreMesh:
        entities = destScene->CreateContentFromSceneDescription(newDesc, false, AttributeChange::Default);
        break;
/*
    case SceneDesc::OgreMesh:
    {
        fs::path path(newDesc.filename.toStdString());
        std::string dirname = path.branch_path().string();

        TundraLogic::SceneImporter importer(destScene);
        Scene::EntityPtr entity = importer.ImportMesh(newDesc.filename.toStdString(), dirname,
            Transform(),std::string(), dest->BaseURL(), AttributeChange::Default, true, std::string(), newDesc);
        if (entity)
            entities << entity.get();
        break;
    }
*/
    case SceneDesc::OgreScene:
    {
        fs::path path(newDesc.filename.toStdString());
        std::string dirname = path.branch_path().string();

        TundraLogic::SceneImporter importer(destScene);
        entities = importer.Import(newDesc.filename.toStdString(), dirname, Transform(),
            dest->BaseURL(), AttributeChange::Default, false/*clearScene*/, false, newDesc);
        break;
    }
    default:
        LogError("Invalid scene description type.");
        break;
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

void AddContentWindow::CheckIfColumnIsEditable(QTreeWidgetItem *item, int column)
{
    if (column == cColumnAssetDestName)
        item->setFlags(item->flags() | Qt::ItemIsEditable);
    else
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);
}

void AddContentWindow::RewriteDestinationNames()
{
    QString storageName = storageComboBox->itemData(storageComboBox->currentIndex()).toString();
    AssetStoragePtr dest;
    bool useDefault = false;
    if (storageName == "DefaultStorage")
    {
        dest = framework->Asset()->GetDefaultAssetStorage();
        useDefault = true;
    }
    else
        dest = framework->Asset()->GetAssetStorage(storageName);

    if (!dest)
    {
        LogError("Could not retrieve asset storage " + storageName.toStdString() + ".");

        // Regenerate storage combo box items to make sure that we're up-to-date.
        storageComboBox->clear();
        foreach(AssetStoragePtr storage, framework->Asset()->GetAssetStorages())
            storageComboBox->addItem(storage->ToString(), storage->Name());

        return;
    }

    assetTreeWidget->setSortingEnabled(false);

    QTreeWidgetItemIterator it(assetTreeWidget);
    while(*it)
    {
        AssetWidgetItem *aitem = dynamic_cast<AssetWidgetItem *>(*it);
        assert(aitem);
        if (aitem && !aitem->isDisabled())
        {
            if (useDefault)
                aitem->desc.destinationName = AssetAPI::ExtractFilenameFromAssetRef(aitem->text(cColumnAssetDestName).trimmed());
            else
                aitem->desc.destinationName = dest->GetFullAssetURL(aitem->text(cColumnAssetDestName).trimmed());
            aitem->setText(cColumnAssetDestName, aitem->desc.destinationName);
        }

        ++it;
    }

    assetTreeWidget->setSortingEnabled(true);
}

void AddContentWindow::HandleUploadCompleted(IAssetUploadTransfer *transfer)
{
    ///\todo update progress bar when the feature is available.
}

void AddContentWindow::HandleUploadFailed(IAssetUploadTransfer *transfer)
{
    ///\todo update progress bar when the feature is available.
}


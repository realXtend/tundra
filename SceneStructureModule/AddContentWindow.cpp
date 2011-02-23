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

#include "NaaliUi.h"
#include "NaaliMainWindow.h"
#include "Framework.h"
#include "AssetAPI.h"
#include "IAssetStorage.h"
#include "IAssetUploadTransfer.h"
#include "SceneDesc.h"
#include "SceneManager.h"
#include "SceneImporter.h"
#include "Transform.h"
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
    scene(dest),
    parentEntities_(0),
    parentAssets_(0),
    contentAdded_(false)
{
    setWindowModality(Qt::ApplicationModal/*Qt::WindowModal*/);
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(" "); //setWindowTitle(tr("Add Content")); imo the add content name is kind of not needed
    QPixmap nullIcon(16,16); // do a null icon to hide the default ugly one
    nullIcon.fill(Qt::transparent);
    setWindowIcon(nullIcon);

    QString widgetStyle = "QProgressBar { border: 1px solid grey; border-radius: 0px; text-align: center; background-color: rgb(244, 244, 244); }" 
        "QProgressBar::chunk { background-color: qlineargradient(spread:pad, x1:0.028, y1:1, x2:0.972, y2:1, stop:0 rgba(194, 194, 194, 100), stop:1 rgba(115, 115, 115, 100)); }";
    setStyleSheet(widgetStyle);

    QFont titleFont("Arial", 16);
    titleFont.setBold(true);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(5, 5, 5, 5);
    setLayout(layout);

    // Entities ui
    parentEntities_ = new QWidget();
    QVBoxLayout *entitiesLayout = new QVBoxLayout();
    entitiesLayout->setContentsMargins(0, 0, 0, 0);
    parentEntities_->setLayout(entitiesLayout);

    QLabel *entityLabel = new QLabel(tr("Entity Creation"));
    entityLabel->setFont(titleFont);
    entityLabel->setStyleSheet("color: rgb(63, 63, 63);");

    entityTreeWidget = new QTreeWidget;
    entityTreeWidget->setColumnCount(3);
    entityTreeWidget->setHeaderLabels(QStringList(QStringList() << tr("Create") << tr("ID") << tr("Name")));
    entityTreeWidget->header()->setResizeMode(QHeaderView::ResizeToContents);
    entityTreeWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);


    QPushButton *selectAllEntitiesButton = new QPushButton(tr("Select All"));
    QPushButton *deselectAllEntitiesButton = new QPushButton(tr("Deselect All"));
    QHBoxLayout *entityButtonsLayout = new QHBoxLayout;
    QSpacerItem *entityButtonSpacer = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    QSpacerItem *middleSpacer = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Fixed);

    QVBoxLayout *entitiesProgressLayout = new QVBoxLayout();
    entitiesStatus_ = new QLabel();
    entitiesProgress_ = new QProgressBar();
    entitiesProgress_->setFixedHeight(20);

    entitiesLayout->addWidget(entityLabel);
    entitiesLayout->addLayout(entitiesProgressLayout);
    entitiesLayout->addWidget(entityTreeWidget);
    entitiesProgressLayout->addWidget(entitiesStatus_);
    entitiesProgressLayout->addWidget(entitiesProgress_);
    entityButtonsLayout->addWidget(selectAllEntitiesButton);
    entityButtonsLayout->addWidget(deselectAllEntitiesButton);
    entityButtonsLayout->addSpacerItem(entityButtonSpacer);
    //entitiesLayout->insertLayout(-1, entitiesProgressLayout);
    entitiesLayout->insertLayout(-1, entityButtonsLayout);
    entitiesLayout->insertSpacerItem(-1, middleSpacer);
    layout->addWidget(parentEntities_);

    // Assets ui
    parentAssets_ = new QWidget();
    QVBoxLayout *assetsLayout = new QVBoxLayout();
    assetsLayout->setContentsMargins(0, 0, 0, 0);
    parentAssets_->setLayout(assetsLayout);

    QLabel *assetLabel = new QLabel(tr("Asset Uploads"));
    assetLabel->setFont(titleFont);
    assetLabel->setStyleSheet("color: rgb(63, 63, 63);");

    assetTreeWidget = new QTreeWidget;
    assetTreeWidget->setColumnCount(5);
    QStringList labels;
    labels << tr("Upload") << tr("Type") << tr("Source name") << tr("Source subname") << tr("Destination name");
    assetTreeWidget->setHeaderLabels(labels);
    assetTreeWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QPushButton *selectAllAssetsButton = new QPushButton(tr("Select All"));
    QPushButton *deselectAllAssetsButton = new QPushButton(tr("Deselect All"));
    QHBoxLayout *assetButtonsLayout = new QHBoxLayout;
    QSpacerItem *assetButtonSpacer = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    QLabel *storageLabel = new QLabel(tr("Asset storage:"));
    storageComboBox = new QComboBox;

    QVBoxLayout *uploadLayout = new QVBoxLayout();
    uploadStatus_ = new QLabel();
    uploadProgress_ = new QProgressBar();
    uploadProgress_->setFixedHeight(20);

    // Get available asset storages. Set default storage selected as default.
    //storageComboBox->addItem(tr("Don't adjust"), "DoNotAdjust");
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

    uploadLayout->addWidget(uploadStatus_);
    uploadLayout->addWidget(uploadProgress_);

    assetsLayout->addWidget(assetLabel);
    assetsLayout->addLayout(uploadLayout);
    assetsLayout->addWidget(assetTreeWidget);
    //assetsLayout->addLayout(uploadLayout);
    assetButtonsLayout->addWidget(selectAllAssetsButton);
    assetButtonsLayout->addWidget(deselectAllAssetsButton);
    assetButtonsLayout->addSpacerItem(assetButtonSpacer);
    assetButtonsLayout->addWidget(storageLabel);
    assetButtonsLayout->addWidget(storageComboBox);
    assetsLayout->insertLayout(-1, assetButtonsLayout);
    layout->addWidget(parentAssets_);

    uploadStatus_->hide();
    uploadProgress_->hide();
    entitiesStatus_->hide();
    entitiesProgress_->hide();

    // General controls
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

    progressStep_ = 0;
    failedUploads_ = 0;
    successfullUploads_ = 0;
    totalUploads_ = 0;
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

void AddContentWindow::showEvent(QShowEvent *e)
{
    CenterToMainWindow();
    QWidget::showEvent(e);
}

void AddContentWindow::AddDescription(const SceneDesc &desc)
{
    sceneDesc = desc;
    AddEntities(desc.entities);
    AddAssets(desc.assets);
    resize(1,1);
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
    if (sceneDesc.type == SceneDesc::AssetUpload && entityDescs.empty())
    {
        SetEntitiesVisible(false);
        return;
    }

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

    // Set a minimum height for our treeview
    int fullHeight = entityTreeWidget->header()->height();
    fullHeight += (entityTreeWidget->sizeHintForRow(0)+5) * entityTreeWidget->model()->rowCount();
    int halfDeskHeight = QApplication::desktop()->screenGeometry().height()/2;
    if(fullHeight<(halfDeskHeight-50))
    {
        entityTreeWidget->setMinimumHeight(fullHeight);
    }
    else
    {
        entityTreeWidget->setMinimumHeight(halfDeskHeight - 100);
    }
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
            aItem->setBackgroundColor(cColumnAssetSourceName, QColor(255,0,0,200));
            aItem->setTextColor(cColumnAssetSourceName, Qt::white);
            aItem->setCheckState(cColumnAssetUpload, Qt::Unchecked);
            aItem->setText(cColumnAssetDestName, "");
            aItem->setDisabled(true);
        }
        else if (res == AssetAPI::FileQueryExternalFile)
        {
            // External reference, mark the item gray and disable it.
            aItem->setBackgroundColor(cColumnAssetSourceName, Qt::gray);
            aItem->setTextColor(cColumnAssetSourceName, Qt::white);
            aItem->setCheckState(cColumnAssetUpload, Qt::Unchecked);
            aItem->setText(cColumnAssetDestName, "");
            aItem->setDisabled(true);
        }
    }

    RewriteDestinationNames();

    // Enable sorting, resize header sections to contents.
    assetTreeWidget->setSortingEnabled(true);
    assetTreeWidget->header()->resizeSections(QHeaderView::ResizeToContents);

    // Set a minimum height for our treeview
    int fullHeight = assetTreeWidget->header()->height();
    fullHeight += (assetTreeWidget->sizeHintForRow(0)+5) * assetTreeWidget->model()->rowCount();
    
    int halfDeskHeight = QApplication::desktop()->screenGeometry().height()/2;
    if(fullHeight<(halfDeskHeight-50))
    {
        assetTreeWidget->setMinimumHeight(fullHeight);
    }
    else
    {
        assetTreeWidget->setMinimumHeight(halfDeskHeight - 100);
    }    

    // Set the windows minumum width from assets tree view
    int minWidth = 10;
    for (int i=0; i<assetTreeWidget->columnCount(); i++)
        minWidth += assetTreeWidget->columnWidth(i);
    setMinimumWidth(minWidth);


    // Sort asset items initially so that erroneous are first
    assetTreeWidget->sortItems(cColumnAssetUpload, Qt::AscendingOrder);
}

void AddContentWindow::RewriteAssetReferences(SceneDesc &sceneDesc, const AssetStoragePtr &dest, bool useDefaultStorage)
{
    QString path(fs::path(sceneDesc.filename.toStdString()).branch_path().string().c_str());

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
                        if (value.isEmpty())
                            continue;

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
                        else
                        {
                            QString ref = AssetAPI::ExtractFilenameFromAssetRef(value);
                            ///\todo Perf hit, find a better way and remove this.
                            AssetAPI::QueryFileLocation(ref, path, value);
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
    uploadStatus_->setText("");
    uploadProgress_->setValue(0);
    entitiesStatus_->setText("Waiting for asset uploads to finish...");
    entitiesProgress_->setValue(0);

    if (CreateNewDesctiption())
    {
        // If no uploads are queued then AddEntities will be called automatically
        if (UploadAssets())
        {        
            if (totalUploads_ > 0)
            {
                uploadStatus_->show();
                uploadProgress_->show();
            }
            if (!newDesc_.entities.empty())
            {
                entitiesStatus_->show();
                entitiesProgress_->show();
            }

            addContentButton->setEnabled(false);
            storageComboBox->setEnabled(false);
        }
        else
            QMessageBox::critical(this, "Uploading", "Starting uploads failed");
    }
    else
        QMessageBox::critical(this, "Uploading", "Starting uploads failed");

}

bool AddContentWindow::CreateNewDesctiption()
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

        return false;
    }
    currentStorage_ = storageName;
    currentStorageBaseUrl_ = dest->BaseURL();

    // Filter which entities will be created and which assets will be uploaded.
    newDesc_ = sceneDesc;
    QTreeWidgetItemIterator eit(entityTreeWidget);
    while(*eit)
    {
        EntityWidgetItem *eitem = dynamic_cast<EntityWidgetItem *>(*eit);
        if (eitem && eitem->checkState(cColumnEntityCreate) == Qt::Unchecked)
        {
            QList<EntityDesc>::const_iterator ei = qFind(newDesc_.entities, eitem->desc);
            if (ei != newDesc_.entities.end())
                newDesc_.entities.removeOne(*ei);
        }

        ++eit;
    }

    // Rewrite components' asset refs
    bool useDefault = storageName == "DefaultStorage";
    RewriteAssetReferences(newDesc_, dest, useDefault);

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
                bool removed = newDesc_.assets.remove(qMakePair(aitem->desc.source, aitem->desc.subname));
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
    QMutableMapIterator<SceneDesc::AssetMapKey, AssetDesc> rewriteIt(newDesc_.assets);
    while(rewriteIt.hasNext())
    {
        rewriteIt.next();
        if (rewriteIt.value().typeName.contains("material", Qt::CaseInsensitive))
            ///\todo This logic will be removed in the future, as we need it generic for any types of assets.
            ReplaceReferences(rewriteIt.value().data, refs);
    }

    return true;
}

bool AddContentWindow::UploadAssets()
{
    AssetStoragePtr dest;
    if (currentStorage_ == "DefaultStorage")
        dest = framework->Asset()->GetDefaultAssetStorage();
    else
        dest = framework->Asset()->GetAssetStorage(currentStorage_);
    if (!dest)
    {
        LogError("Could not retrieve asset storage " + currentStorage_.toStdString() + ".");
        // Regenerate storage combo box items to make sure that we're up-to-date.
        storageComboBox->clear();
        foreach(AssetStoragePtr storage, framework->Asset()->GetAssetStorages())
            storageComboBox->addItem(storage->ToString(), storage->Name());
        return false;
    }

    Scene::ScenePtr destScene = scene.lock();
    if (!destScene)
        return false;

    // Upload
    if (!newDesc_.assets.empty())
    {
        LogDebug("Starting uploading of " + ToString(newDesc_.assets.size()) + " asset" + "(s).");

        totalUploads_ = 0;
        progressStep_ = 0;
        failedUploads_ = 0;
        successfullUploads_ = 0;
        foreach(AssetDesc ad, newDesc_.assets)
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
                    if (connect(transfer.get(), SIGNAL(Completed(IAssetUploadTransfer *)), SLOT(HandleUploadCompleted(IAssetUploadTransfer *)), Qt::UniqueConnection) &&
                        connect(transfer.get(), SIGNAL(Failed(IAssetUploadTransfer *)), SLOT(HandleUploadFailed(IAssetUploadTransfer *)), Qt::UniqueConnection))
                        totalUploads_++;
                }
            }
            catch(const Exception &e)
            {
                LogError(std::string(e.what()));
            }
        }
        if (totalUploads_ > 0)
            progressStep_ = 100 / totalUploads_;
        else
            progressStep_ = 100;
    }
    else
        AddEntities(); // if no uploads, then add entities now!
    return true;
}

void AddContentWindow::AddEntities()
{
    AssetStoragePtr dest;
    if (currentStorage_ == "DefaultStorage")
        dest = framework->Asset()->GetDefaultAssetStorage();
    else
        dest = framework->Asset()->GetAssetStorage(currentStorage_);
    if (!dest)
    {
        LogError("Could not retrieve asset storage " + currentStorage_.toStdString() + ".");
        // Regenerate storage combo box items to make sure that we're up-to-date.
        storageComboBox->clear();
        foreach(AssetStoragePtr storage, framework->Asset()->GetAssetStorages())
            storageComboBox->addItem(storage->ToString(), storage->Name());
        return;
    }

    Scene::ScenePtr destScene = scene.lock();
    if (!destScene)
    {
        LogError("Could not add entities to scene, no scene ptr!");
        return;
    }

    if (!newDesc_.entities.empty())
    {
        // Create entities.
        QList<Scene::Entity *> entities;
        switch(newDesc_.type)
        {
        case SceneDesc::Naali:
        case SceneDesc::OgreMesh:
            entities = destScene->CreateContentFromSceneDesc(newDesc_, false, AttributeChange::Default);
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
            fs::path path(newDesc_.filename.toStdString());
            std::string dirname = path.branch_path().string();

            TundraLogic::SceneImporter importer(destScene);
            entities = importer.Import(newDesc_.filename.toStdString(), dirname, Transform(),
                dest->BaseURL(), AttributeChange::Default, false/*clearScene*/, false);
            break;
        }
        case SceneDesc::AssetUpload:
            break;
        default:
            LogError("Invalid scene description type.");
            break;
        }

        if (!entities.empty())
        {
            entitiesStatus_->setText(QString("Added %1 entities to scene successfully").arg(entities.count()));
            entitiesStatus_->setText(QString("%1/%2 entities created succesfully").arg(QString::number(entities.count()), QString::number(newDesc_.entities.count())));
            if (position != Vector3df())
                SceneStructureModule::CentralizeEntitiesTo(position, entities);
        }
        else
        {
            entitiesStatus_->setText("No entities were created, even if the intent was to create " + QString::number(newDesc_.entities.count()));
            QMessageBox::warning(this, "Entity Creation", "No entities were created, even if input had entities!");
            return;
        }

        entitiesProgress_->setValue(100);
    }
    else
        entitiesStatus_->setText("No entities were selected for add");
    
    contentAdded_ = true; // this will be emitted on Close() for those who are interested
    cancelButton->setText(tr("Close"));
    addContentButton->setEnabled(true);
    storageComboBox->setEnabled(true);
}

void AddContentWindow::CenterToMainWindow()
{
    // Center the window to main window
    if (framework->Ui()->MainWindow())
    {
        QRect mainRect = framework->Ui()->MainWindow()->rect();
        QPoint mainPos = framework->Ui()->MainWindow()->pos();
        QPoint mainCenter = mainPos + mainRect.center();
        if((mainCenter.y() - height()/2)>=0)
        {
            move(mainCenter.x() - width()/2, mainCenter.y() - height()/2);
        }
        else
        {
            move(mainCenter.x() - width()/2, 0);
        }
    }
}

void AddContentWindow::SetEntitiesVisible(bool visible)
{
    if (parentEntities_)
    {
        parentEntities_->setVisible(visible);
        resize(width(), 200);
        CenterToMainWindow();
    }
}

void AddContentWindow::SetAssetsVisible(bool visible)
{
    if (parentAssets_)
    {
        parentAssets_->setVisible(visible);
        resize(width(), 200);
        CenterToMainWindow();
    }
}

void AddContentWindow::Close()
{
    emit Completed(contentAdded_, currentStorageBaseUrl_);
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
    successfullUploads_++;
    uploadStatus_->setText("Uploaded " + transfer->AssetRef());
    uploadProgress_->setValue(uploadProgress_->value() + progressStep_);
    UpdateUploadStatus(true, transfer->AssetRef());
    CheckUploadTotals();
}

void AddContentWindow::HandleUploadFailed(IAssetUploadTransfer *transfer)
{
    failedUploads_++;
    uploadStatus_->setText("Upload failed for " + transfer->AssetRef());
    uploadProgress_->setValue(uploadProgress_->value() + progressStep_);
    UpdateUploadStatus(false, transfer->AssetRef());
    CheckUploadTotals();
}

void AddContentWindow::UpdateUploadStatus(bool succesfull, const QString &assetRef)
{
    QColor statusColor;
    if (succesfull)
        statusColor = QColor(0, 255, 0, 75);
    else
        statusColor = QColor(255, 0, 0, 75);

    QTreeWidgetItemIterator it(assetTreeWidget);
    while(*it)
    {
        AssetWidgetItem *aitem = dynamic_cast<AssetWidgetItem *>(*it);
        assert(aitem);
        if (aitem && !aitem->isDisabled())
        {
            if (aitem->text(cColumnAssetDestName) == assetRef)
            {
                aitem->setBackgroundColor(cColumnAssetTypeName, statusColor);
                aitem->setBackgroundColor(cColumnAssetSourceName, statusColor);
                aitem->setBackgroundColor(cColumnAssetSubname, statusColor);
                aitem->setBackgroundColor(cColumnAssetDestName, statusColor);
                break;
            }
        }
        ++it;
    }
}

void AddContentWindow::CheckUploadTotals()
{
    int totalNow = successfullUploads_ + failedUploads_;
    if (totalNow == totalUploads_)
    {
        uploadStatus_->setText(QString("%1/%2 uploads completed succesfully").arg(QString::number(successfullUploads_), QString::number(totalUploads_)));
        AddEntities();
    }
}

